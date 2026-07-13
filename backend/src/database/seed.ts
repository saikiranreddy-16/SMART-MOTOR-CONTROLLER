import 'dotenv/config';
import { NestFactory } from '@nestjs/core';
import { AppModule } from '../app.module';
import { DataSource } from 'typeorm';
import { User } from './entities/user.entity';
import { UserRole } from './entities/user-role.enum';
import { Device, PumpTypeEnum } from './entities/device.entity';
import { Member } from './entities/member.entity';
import { Motor } from './entities/motor.entity';
import { WaterLevel } from './entities/water-level.entity';
import * as bcrypt from 'bcrypt';

async function bootstrap() {
  console.log('Seeding database with sample data...');
  const app = await NestFactory.createApplicationContext(AppModule);
  const dataSource = app.get<DataSource>('DataSource');

  // Clear existing data (in reverse order of foreign keys)
  await dataSource.query(
    'TRUNCATE TABLE device_logs, telemetry_logs, pump_history, device_members, water_levels, motors, devices, users, weather_cache, ota_updates, notifications CASCADE;',
  );

  const passwordHash = await bcrypt.hash('password123', 10);

  // 1. Seed Users
  const farmer = new User();
  farmer.name = 'Rajesh Kumar';
  farmer.phoneNumber = '+919876543210';
  farmer.passwordHash = passwordHash;
  farmer.role = UserRole.FARMER;
  await dataSource.manager.save(farmer);

  const memberUser = new User();
  memberUser.name = 'Kiran Kumar';
  memberUser.phoneNumber = '+919111222333';
  memberUser.passwordHash = passwordHash;
  memberUser.role = UserRole.MEMBER;
  await dataSource.manager.save(memberUser);

  const installer = new User();
  installer.name = 'Srinivas Rao';
  installer.phoneNumber = '+919222333444';
  installer.passwordHash = passwordHash;
  installer.role = UserRole.INSTALLER;
  await dataSource.manager.save(installer);

  // 2. Seed Device
  const device = new Device();
  device.id = 'ESP32_A83C10';
  device.name = 'Borewell Pump 1';
  device.simPhoneNumber = '+919988776655';
  device.simImsi = '404450998877665';
  device.pumpType = PumpTypeEnum.AC_THREE;
  device.activeLanguage = 1;
  device.owner = farmer;
  device.safetyThresholds = {
    underVoltageLimit: 180,
    overVoltageLimit: 260,
    imbalancePercent: 15,
    dryRunCurrentMultiplier: 0.6,
    dryRunCooldownMinutes: 45,
    overcurrentNominalAmps: 15,
  };
  device.pairedRemoteId = 1419995410;
  await dataSource.manager.save(device);

  // 3. Seed Whitelisted Members
  const memberObj = new Member();
  memberObj.deviceId = device.id;
  memberObj.name = 'Kiran Kumar';
  memberObj.phoneNumber = '+919111222333';
  memberObj.role = UserRole.MEMBER;
  await dataSource.manager.save(memberObj);

  const workerObj = new Member();
  workerObj.deviceId = device.id;
  workerObj.name = 'Ramu';
  workerObj.phoneNumber = '+919444555666';
  workerObj.role = UserRole.MEMBER; // fallback role
  await dataSource.manager.save(workerObj);

  // 4. Seed Motor Parameters
  const motor = new Motor();
  motor.deviceId = device.id;
  motor.nominalCurrent = 12.0;
  motor.motorModel = 'V4 Submersible';
  motor.brand = 'Texmo';
  await dataSource.manager.save(motor);

  // 5. Seed Water Level Parameters
  const waterLevel = new WaterLevel();
  waterLevel.deviceId = device.id;
  waterLevel.sensorType = 'HYDROSTATIC_PROBE';
  waterLevel.maxDepthMeters = 10.0;
  waterLevel.currentDepthPercent = 85.0;
  waterLevel.warningThresholdPercent = 20.0;
  waterLevel.criticalThresholdPercent = 10.0;
  await dataSource.manager.save(waterLevel);

  console.log('Database Seeding Successful!');
  await app.close();
}

bootstrap().catch((err) => {
  console.error('Seed execution failed:', err);
  process.exit(1);
});
