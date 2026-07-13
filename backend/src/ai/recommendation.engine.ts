import { Injectable } from '@nestjs/common';
import { WeatherService } from './weather.service';
import { IrrigationEngine } from './irrigation.engine';

@Injectable()
export class RecommendationEngine {
  constructor(
    private readonly weatherService: WeatherService,
    private readonly irrigationEngine: IrrigationEngine,
  ) {}

  // Generate irrigation runtime recommendations for the farmer
  async generateDailyAdvice(
    latitude: number,
    longitude: number,
    currentMoisture: number,
    soilType: string,
  ): Promise<{ recommendedDuration: number; advice: string }> {
    // 1. Fetch weather forecast
    const forecast = await this.weatherService.getForecast(latitude, longitude);

    // 2. Compute moisture deficit
    const deficit = this.irrigationEngine.calculateWaterDeficit(currentMoisture, soilType);

    // 3. Formulate recommendation
    if (forecast.rainProbability > 65) {
      return {
        recommendedDuration: 0,
        advice: `Rain probability is high (${forecast.rainProbability}%). Postpone irrigation to save water and prevent soil waterlogging.`,
      };
    }

    if (deficit > 15) {
      // Calculate duration: 2 minutes of run time per 1% deficit (placeholder calibration)
      const duration = Math.round(deficit * 2.0);
      return {
        recommendedDuration: duration,
        advice: `Soil is dry (Deficit: ${deficit.toFixed(0)}%). Run motor for ${duration} minutes to reach target moisture.`,
      };
    }

    return {
      recommendedDuration: 0,
      advice: 'Soil moisture is optimal. Irrigation not required today.',
    };
  }
}
