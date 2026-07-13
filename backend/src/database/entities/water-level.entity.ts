import { Entity, PrimaryGeneratedColumn, Column, OneToOne, JoinColumn, Index } from 'typeorm';
import { Device } from './device.entity';

@Entity('water_levels')
export class WaterLevel {
  @PrimaryGeneratedColumn('uuid')
  id: string;

  @Index()
  @Column()
  deviceId: string;

  @Column({ default: 'FLOAT_SWITCH' })
  sensorType: string; // 'FLOAT_SWITCH' or 'HYDROSTATIC_PROBE'

  @Column({ type: 'real', default: 5.0 })
  maxDepthMeters: number;

  @Column({ type: 'real', default: 100.0 })
  currentDepthPercent: number;

  @Column({ type: 'real', default: 20.0 })
  warningThresholdPercent: number;

  @Column({ type: 'real', default: 10.0 })
  criticalThresholdPercent: number;

  @OneToOne(() => Device, { onDelete: 'CASCADE' })
  @JoinColumn({ name: 'deviceId' })
  device: Device;
}
