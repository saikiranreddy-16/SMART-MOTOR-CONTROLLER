import { Module } from '@nestjs/common';
import { MotorController } from './motor.controller';
import { MqttModule } from '../mqtt/mqtt.module';

@Module({
  imports: [MqttModule],
  controllers: [MotorController],
})
export class MotorModule {}
