import { Injectable, Inject, Logger } from '@nestjs/common';
import { DataSource } from 'typeorm';
import { WeatherCache } from '../database/entities/weather-cache.entity';
import axios from 'axios';

@Injectable()
export class WeatherService {
  private readonly logger = new Logger('WeatherService');

  constructor(@Inject('DataSource') private readonly dataSource: DataSource) {}

  // Fetch forecast, utilizing cache to avoid rate limits
  async getForecast(latitude: number, longitude: number): Promise<{ rainProbability: number; description: string }> {
    const cacheRepo = this.dataSource.getRepository(WeatherCache);
    const twoHoursAgo = new Date(Date.now() - 2 * 60 * 60 * 1000);

    // Look for matching cache item within 2 hours
    try {
      const cached = await cacheRepo
        .createQueryBuilder('cache')
        .where('ABS(cache.latitude - :lat) < 0.02', { lat: latitude })
        .andWhere('ABS(cache.longitude - :lon) < 0.02', { lon: longitude })
        .andWhere('cache.fetchedAt > :time', { time: twoHoursAgo })
        .orderBy('cache.fetchedAt', 'DESC')
        .getOne();

      if (cached) {
        this.logger.log(`Serving cached weather for coordinates (${latitude}, ${longitude})`);
        return {
          rainProbability: cached.rainProbability,
          description: cached.description,
        };
      }
    } catch (err) {
      this.logger.error('Failed to read weather cache database:', err.message);
    }

    // Call external Open-Meteo API
    this.logger.log(`Calling weather service for coordinates (${latitude}, ${longitude})`);
    try {
      const url = `https://api.open-meteo.com/v1/forecast?latitude=${latitude}&longitude=${longitude}&hourly=precipitation_probability,temperature_2m&forecast_days=1`;
      const response = await axios.get(url);

      const probabilities: number[] = response.data?.hourly?.precipitation_probability || [];
      const maxRainProb = probabilities.length > 0 ? Math.max(...probabilities) : 0;

      let description = 'Sunny';
      if (maxRainProb > 70) description = 'Rain Expected';
      else if (maxRainProb > 30) description = 'Partly Cloudy';

      // Write cache to database
      const cache = new WeatherCache();
      cache.latitude = latitude;
      cache.longitude = longitude;
      cache.rainProbability = maxRainProb;
      cache.description = description;
      await cacheRepo.save(cache);

      return { rainProbability: maxRainProb, description };
    } catch (err) {
      this.logger.error('Weather forecast fetch failed, using default fallback', err.message);
      return { rainProbability: 10, description: 'Sunny' };
    }
  }
}
