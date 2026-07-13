import { Module } from '@nestjs/common';
import { WeatherService } from './weather.service';
import { IrrigationEngine } from './irrigation.engine';
import { PredictionEngine } from './prediction.engine';
import { RecommendationEngine } from './recommendation.engine';

@Module({
  providers: [WeatherService, IrrigationEngine, PredictionEngine, RecommendationEngine],
  exports: [WeatherService, IrrigationEngine, PredictionEngine, RecommendationEngine],
})
export class AiModule {}
