import { Entity, PrimaryGeneratedColumn, Column, OneToOne, JoinColumn, Index } from 'typeorm';
import { Device } from './device.entity';

@Entity('motors')
export class Motor {
  @PrimaryGeneratedColumn('uuid')
  id: string;

  @Index()
  @Column()
  deviceId: string;

  @Column({ type: 'real', default: 12.0 })
  nominalCurrent: number;

  @Column({ default: 'AC Induction Motor' })
  motorModel: string;

  @Column({ default: 'Texmo 5HP' })
  brand: string;

  @OneToOne(() => Device, { onDelete: 'CASCADE' })
  @JoinColumn({ name: 'deviceId' })
  device: Device;
}
