import { Entity, PrimaryGeneratedColumn, Column, CreateDateColumn, ManyToOne, Index } from 'typeorm';
import { Device } from './device.entity';

export enum LogCategory {
  FAULT = 'FAULT',
  USER_ACTION = 'USER_ACTION',
  SECURITY = 'SECURITY',
}

@Entity('device_logs')
export class DeviceLog {
  @PrimaryGeneratedColumn({ type: 'bigint' })
  id: string;

  @Index()
  @Column()
  deviceId: string;

  @Index()
  @CreateDateColumn()
  timestamp: Date;

  @Column({
    type: 'enum',
    enum: LogCategory,
    default: LogCategory.USER_ACTION,
  })
  category: LogCategory;

  @Column()
  eventName: string; // e.g. "DRY_RUN", "START_PUMP"

  @Column({ type: 'text', nullable: true })
  description: string;

  @Column()
  triggeredBy: string; // UID or "SYSTEM"

  @ManyToOne(() => Device, (device) => device.logs, { onDelete: 'CASCADE' })
  device: Device;
}
