import { Injectable, OnModuleInit, Inject, Logger, OnModuleDestroy } from '@nestjs/common';
import * as mqtt from 'mqtt';
import Redis from 'ioredis';
import { DataSource } from 'typeorm';
import { TelemetryLog } from '../database/entities/telemetry-log.entity';
import { DeviceLog, LogCategory } from '../database/entities/device-log.entity';
import { Device } from '../database/entities/device.entity';
import { WaterLevel } from '../database/entities/water-level.entity';
import { DeviceGateway } from './device.gateway';

@Injectable()
export class MqttService implements OnModuleInit, OnModuleDestroy {
  private readonly logger = new Logger('MqttService');
  private mqttClient: mqtt.MqttClient;
  private redis: Redis;

  constructor(
    @Inject('DataSource') private readonly dataSource: DataSource,
    private readonly deviceGateway: DeviceGateway,
  ) {
    this.redis = new Redis({
      host: process.env.REDIS_HOST || 'localhost',
      port: parseInt(process.env.REDIS_PORT) || 6379,
    });
  }

  onModuleInit() {
    const host = process.env.MQTT_HOST || 'localhost';
    const port = process.env.MQTT_PORT || '1883';
    const username = process.env.MQTT_USERNAME || '';
    const password = process.env.MQTT_PASSWORD || '';
    const brokerUrl = `mqtt://${host}:${port}`;

    this.logger.log(`Connecting to MQTT broker at ${brokerUrl}`);

    this.mqttClient = mqtt.connect(brokerUrl, {
      username: username || undefined,
      password: password || undefined,
      reconnectPeriod: 5000,
    });

    this.mqttClient.on('connect', () => {
      this.logger.log('Successfully connected to MQTT Broker.');

      // Subscribe to standard device topics
      this.mqttClient.subscribe('device/+/status', (err) => {
        if (err) this.logger.error('Failed to subscribe to device status');
      });
      this.mqttClient.subscribe('device/+/telemetry', (err) => {
        if (err) this.logger.error('Failed to subscribe to device telemetry');
      });
      this.mqttClient.subscribe('device/+/waterlevel', (err) => {
        if (err) this.logger.error('Failed to subscribe to device water level');
      });
      this.mqttClient.subscribe('device/+/ota', (err) => {
        if (err) this.logger.error('Failed to subscribe to device ota');
      });
    });

    this.mqttClient.on('close', () => {
      this.logger.warn('MQTT connection closed. Reconnecting...');
    });

    this.mqttClient.on('message', async (topic, message) => {
      try {
        const payload = JSON.parse(message.toString());
        const topicParts = topic.split('/');
        const deviceId = topicParts[1];
        const subTopic = topicParts[2];

        this.logger.debug(`MQTT [${topic}] Message: ${message.toString()}`);

        switch (subTopic) {
          case 'status':
            await this.handleStatus(deviceId, payload);
            break;
          case 'telemetry':
            await this.handleTelemetry(deviceId, payload);
            break;
          case 'waterlevel':
            await this.handleWaterLevel(deviceId, payload);
            break;
          case 'ota':
            await this.handleOtaStatus(deviceId, payload);
            break;
        }
      } catch (err) {
        this.logger.error(`Error processing MQTT topic message [${topic}]: ${err.message}`);
      }
    });
  }

  onModuleDestroy() {
    if (this.mqttClient) {
      this.mqttClient.end();
    }
  }

  // 1. Handle status notifications (online/offline)
  private async handleStatus(deviceId: string, payload: any) {
    const isOnline = payload.status === 'ONLINE';
    this.logger.log(`Device ${deviceId} is now ${payload.status}`);

    const deviceRepo = this.dataSource.getRepository(Device);
    await deviceRepo.update(deviceId, { isOnline });

    // Cache online state
    await this.redis.set(`device:${deviceId}:status`, payload.status, 'EX', 120);

    // Broadcast update to WebSockets
    this.deviceGateway.broadcastDeviceUpdate(deviceId, { type: 'STATUS', isOnline, status: payload.status });
  }

  // 2. Handle telemetry updates and trigger safety warnings
  private async handleTelemetry(deviceId: string, data: any) {
    // Cache latest telemetry in Redis
    await this.redis.set(`device:${deviceId}:telemetry`, JSON.stringify(data), 'EX', 60);

    // Save to Postgres
    const telemetry = new TelemetryLog();
    telemetry.deviceId = deviceId;
    telemetry.timestamp = new Date((data.timestamp || Date.now() / 1000) * 1000);
    telemetry.voltageR = data.voltage?.R || 0;
    telemetry.voltageY = data.voltage?.Y || 0;
    telemetry.voltageB = data.voltage?.B || 0;
    telemetry.currentR = data.current?.R || 0;
    telemetry.currentY = data.current?.Y || 0;
    telemetry.currentB = data.current?.B || 0;
    telemetry.waterLevel = data.sensors?.water_level || 0;
    telemetry.soilMoisture = data.sensors?.soil_moisture || 0;
    telemetry.casingTemp = data.sensors?.casing_temp || 0;
    telemetry.powerAvailable = data.power_available ?? true;

    await this.dataSource.getRepository(TelemetryLog).save(telemetry);

    // Handle high-priority fault alerts sent in telemetry
    if (data.fault_trip) {
      const log = new DeviceLog();
      log.deviceId = deviceId;
      log.category = LogCategory.FAULT;
      log.eventName = data.fault_trip.code || 'FAULT_TRIP';
      log.description = data.fault_trip.message || 'Motor tripped due to safety hazard';
      log.triggeredBy = 'SYSTEM';
      await this.dataSource.getRepository(DeviceLog).save(log);

      this.deviceGateway.broadcastDeviceUpdate(deviceId, {
        type: 'ALERT',
        code: log.eventName,
        description: log.description,
      });
    }

    // Broadcast to WebSocket clients
    this.deviceGateway.broadcastDeviceUpdate(deviceId, { type: 'TELEMETRY', telemetry });
  }

  // 3. Handle water level sensors
  private async handleWaterLevel(deviceId: string, data: any) {
    const levelRepo = this.dataSource.getRepository(WaterLevel);
    await levelRepo.update({ deviceId }, { currentDepthPercent: data.percent });

    this.deviceGateway.broadcastDeviceUpdate(deviceId, { type: 'WATER_LEVEL', percent: data.percent });
  }

  // 4. Handle OTA states
  private async handleOtaStatus(deviceId: string, data: any) {
    const log = new DeviceLog();
    log.deviceId = deviceId;
    log.category = LogCategory.USER_ACTION;
    log.eventName = 'OTA_PROGRESS';
    log.description = `OTA Update progress: ${data.state} (${data.percent || 0}%)`;
    log.triggeredBy = 'SYSTEM';
    await this.dataSource.getRepository(DeviceLog).save(log);

    this.deviceGateway.broadcastDeviceUpdate(deviceId, { type: 'OTA', state: data.state, percent: data.percent });
  }

  // 5. Dispatch commands (start/stop)
  publishCommand(deviceId: string, command: any) {
    const topic = `device/${deviceId}/command`;
    this.mqttClient.publish(topic, JSON.stringify(command), { qos: 1 });
    this.logger.log(`Dispatched MQTT command to device ${deviceId} on topic ${topic}`);
  }
}
