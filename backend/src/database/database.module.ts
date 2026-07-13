import { Module, Global } from '@nestjs/common';
import { DataSource } from 'typeorm';
import { User } from './entities/user.entity';
import { Device } from './entities/device.entity';
import { Member } from './entities/member.entity';
import { DeviceLog } from './entities/device-log.entity';
import { PumpHistory } from './entities/pump-history.entity';
import { TelemetryLog } from './entities/telemetry-log.entity';
import { Notification } from './entities/notification.entity';
import { WeatherCache } from './entities/weather-cache.entity';
import { OtaUpdate } from './entities/ota-update.entity';
import { Motor } from './entities/motor.entity';
import { WaterLevel } from './entities/water-level.entity';

@Global()
@Module({
  providers: [
    {
      provide: 'DataSource',
      useFactory: async () => {
        const dataSource = new DataSource({
          type: 'postgres',
          host: process.env.DATABASE_HOST || 'localhost',
          port: parseInt(process.env.DATABASE_PORT) || 5432,
          username: process.env.DATABASE_USERNAME || 'postgres',
          password: process.env.DATABASE_PASSWORD || 'secretpassword123',
          database: process.env.DATABASE_NAME || 'smartfarm',
          entities: [
            User,
            Device,
            Member,
            DeviceLog,
            PumpHistory,
            TelemetryLog,
            Notification,
            WeatherCache,
            OtaUpdate,
            Motor,
            WaterLevel,
          ],
          synchronize: process.env.NODE_ENV !== 'production',
        });
        return dataSource.initialize();
      },
    },
  ],
  exports: ['DataSource'],
})
export class DatabaseModule {}
