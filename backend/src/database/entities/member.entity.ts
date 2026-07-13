import { Entity, PrimaryGeneratedColumn, Column, ManyToOne, Index, Unique } from 'typeorm';
import { Device } from './device.entity';
import { UserRole } from './user-role.enum';

@Entity('device_members')
@Unique(['deviceId', 'phoneNumber'])
export class Member {
  @PrimaryGeneratedColumn('uuid')
  id: string;

  @Index()
  @Column()
  deviceId: string;

  @Column()
  phoneNumber: string;

  @Column()
  name: string;

  @Column({
    type: 'enum',
    enum: UserRole,
    default: UserRole.MEMBER,
  })
  role: UserRole;

  @ManyToOne(() => Device, (device) => device.members, { onDelete: 'CASCADE' })
  device: Device;
}
