import { Entity, PrimaryColumn, Column, CreateDateColumn, ManyToOne, OneToMany, Index } from 'typeorm';
import { User } from './user.entity';
import { Member } from './member.entity';
import { DeviceLog } from './device-log.entity';
import { PumpHistory } from './pump-history.entity';
import { TelemetryLog } from './telemetry-log.entity';

export enum PumpTypeEnum {
  AC_SINGLE = 'AC_SINGLE_PHASE',
  AC_THREE = 'AC_THREE_PHASE',
  SOLAR_VFD = 'SOLAR_VFD',
}

@Entity('devices')
export class Device {
  @PrimaryColumn()
  id: string; // ESP32 MAC ID

  @Column()
  name: string;

  @Column({ nullable: true })
  simPhoneNumber: string;

  @Column({ nullable: true })
  simImsi: string;

  @Column({
    type: 'enum',
    enum: PumpTypeEnum,
    default: PumpTypeEnum.AC_THREE,
  })
  pumpType: PumpTypeEnum;

  @Column({ default: 1 }) // 1 = ENG, 2 = TEL, 3 = HIN
  activeLanguage: number;

  @Column({ default: false })
  isOnline: boolean;

  @Column({ type: 'jsonb', default: {} })
  safetyThresholds: {
    underVoltageLimit: number;
    overVoltageLimit: number;
    imbalancePercent: number;
    dryRunCurrentMultiplier: number;
    dryRunCooldownMinutes: number;
    overcurrentNominalAmps: number;
  };

  @Column({ type: 'bigint', nullable: true })
  pairedRemoteId: number;

  @Index()
  @Column({ nullable: true })
  ownerId: string;

  @CreateDateColumn()
  registeredAt: Date;

  @ManyToOne(() => User, (user) => user.devices, { onDelete: 'SET NULL' })
  owner: User;

  @OneToMany(() => Member, (member) => member.device)
  members: Member[];

  @OneToMany(() => DeviceLog, (log) => log.device)
  logs: DeviceLog[];

  @OneToMany(() => PumpHistory, (history) => history.device)
  pumpHistory: PumpHistory[];

  @OneToMany(() => TelemetryLog, (telemetry) => telemetry.device)
  telemetryLogs: TelemetryLog[];
}
