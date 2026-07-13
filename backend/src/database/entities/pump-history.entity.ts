import { Entity, PrimaryGeneratedColumn, Column, ManyToOne, Index } from 'typeorm';
import { Device } from './device.entity';

@Entity('pump_history')
export class PumpHistory {
  @PrimaryGeneratedColumn('uuid')
  id: string;

  @Index()
  @Column()
  deviceId: string;

  @Column()
  startedAt: Date;

  @Column({ nullable: true })
  stoppedAt: Date;

  @Column({ type: 'int', nullable: true })
  durationMinutes: number;

  @Column()
  triggeredBy: string;

  @Column({ default: 'SUCCESS' })
  status: string; // e.g. "SUCCESS", "FAULT_TRIP"

  @ManyToOne(() => Device, (device) => device.pumpHistory, { onDelete: 'CASCADE' })
  device: Device;
}
