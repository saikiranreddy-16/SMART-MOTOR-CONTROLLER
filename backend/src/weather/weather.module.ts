import { Module } from '@nestjs/common';
import { WeatherController } from './weather.controller';
import { AiModule } from '../ai/ai.module';

@Module({
  imports: [AiModule],
  controllers: [WeatherController],
})
export class WeatherModule {}
