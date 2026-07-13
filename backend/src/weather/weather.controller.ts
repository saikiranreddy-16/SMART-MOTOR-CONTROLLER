import { Controller, Get, Query, UseGuards } from '@nestjs/common';
import { JwtAuthGuard } from '../auth/jwt-auth.guard';
import { RecommendationEngine } from '../ai/recommendation.engine';
import { WeatherService } from '../ai/weather.service';
import { ApiTags, ApiOperation, ApiBearerAuth } from '@nestjs/swagger';

@ApiTags('Weather & AI Advice')
@ApiBearerAuth()
@UseGuards(JwtAuthGuard)
@Controller('weather')
export class WeatherController {
  constructor(
    private readonly weatherService: WeatherService,
    private readonly recommendationEngine: RecommendationEngine,
  ) {}

  @Get('forecast')
  @ApiOperation({ summary: 'Retrieves weather forecast for coordinates' })
  async getForecast(@Query('lat') lat: number, @Query('lon') lon: number) {
    return this.weatherService.getForecast(lat, lon);
  }

  @Get('advice')
  @ApiOperation({ summary: 'Generates AI irrigation recommendation for the farmer' })
  async getAdvice(
    @Query('lat') lat: number,
    @Query('lon') lon: number,
    @Query('moisture') moisture: number,
    @Query('soilType') soilType: string,
  ) {
    return this.recommendationEngine.generateDailyAdvice(lat, lon, moisture, soilType);
  }
}
