import { Entity, PrimaryGeneratedColumn, Column, CreateDateColumn, Index } from 'typeorm';

@Entity('weather_cache')
@Index(['latitude', 'longitude'])
export class WeatherCache {
  @PrimaryGeneratedColumn()
  id: number;

  @Column({ type: 'real' })
  latitude: number;

  @Column({ type: 'real' })
  longitude: number;

  @Column({ type: 'real' })
  rainProbability: number;

  @Column()
  description: string;

  @CreateDateColumn()
  fetchedAt: Date;
}
