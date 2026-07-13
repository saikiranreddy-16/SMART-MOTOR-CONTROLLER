import { Entity, PrimaryGeneratedColumn, Column, ManyToOne, Index } from 'typeorm';
import { Device } from './device.entity';

@Entity('telemetry_logs')
@Index(['deviceId', 'timestamp'])
export class TelemetryLog {
  @PrimaryGeneratedColumn({ type: 'bigint' })
  id: string;

  @Column()
  deviceId: string;

  @Column()
  timestamp: Date;

  @Column({ type: 'real', nullable: true })
  voltageR: number;

  @Column({ type: 'real', nullable: true })
  voltageY: number;

  @Column({ type: 'real', nullable: true })
  voltageB: number;

  @Column({ type: 'real', nullable: true })
  currentR: number;

  @Column({ type: 'real', nullable: true })
  currentY: number;

  @Column({ type: 'real', nullable: true })
  currentB: number;

  @Column({ type: 'real', nullable: true })
  waterLevel: number;

  @Column({ type: 'real', nullable: true })
  soilMoisture: number;

  @Column({ type: 'real', nullable: true })
  casingTemp: number;

  @Column({ default: true })
  powerAvailable: boolean;

  @ManyToOne(() => Device, (device) => device.telemetryLogs, { onDelete: 'CASCADE' })
  device: Device;
}
