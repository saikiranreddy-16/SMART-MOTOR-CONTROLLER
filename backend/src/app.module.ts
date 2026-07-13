import { Module, NestModule, MiddlewareConsumer } from '@nestjs/common';
import { DataSource } from 'typeorm';
import { ConfigModule } from '@nestjs/config';
import { validate } from './common/env.validation';
import { DatabaseModule } from './database/database.module';
import { AuthModule } from './auth/auth.module';
import { AiModule } from './ai/ai.module';
import { MqttModule } from './mqtt/mqtt.module';
import { MotorModule } from './motor/motor.module';
import { MembersModule } from './members/members.module';
import { WeatherModule } from './weather/weather.module';
import { NotificationsModule } from './notifications/notifications.module';
import { OtaModule } from './ota/ota.module';
import { HealthController } from './common/health/health.controller';
import { LogsController } from './common/logs.controller';
import { RequestIdMiddleware } from './common/middleware/request-id.middleware';

@Module({
  imports: [
    ConfigModule.forRoot({ isGlobal: true, validate }),
    DatabaseModule,
    AuthModule,
    AiModule,
    MqttModule,
    MotorModule,
    MembersModule,
    WeatherModule,
    NotificationsModule,
    OtaModule,
  ],
  controllers: [HealthController, LogsController],
})
export class AppModule implements NestModule {
  configure(consumer: MiddlewareConsumer) {
    consumer.apply(RequestIdMiddleware).forRoutes('*');
  }
}
