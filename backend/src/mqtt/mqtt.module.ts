import { Module, Global } from '@nestjs/common';
import { MqttService } from './mqtt.service';
import { DeviceGateway } from './device.gateway';

@Global()
@Module({
  providers: [MqttService, DeviceGateway],
  exports: [MqttService, DeviceGateway],
})
export class MqttModule {}
