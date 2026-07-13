import { Entity, PrimaryGeneratedColumn, Column, CreateDateColumn, OneToMany } from 'typeorm';
import { Device } from './device.entity';
import { UserRole } from './user-role.enum';
export { UserRole };

@Entity('users')
export class User {
  @PrimaryGeneratedColumn('uuid')
  id: string;

  @Column()
  name: string;

  @Column({ unique: true })
  phoneNumber: string;

  @Column({ nullable: true })
  email: string;

  @Column()
  passwordHash: string;

  @Column({
    type: 'enum',
    enum: UserRole,
    default: UserRole.MEMBER,
  })
  role: UserRole;

  @Column({ nullable: true })
  fcmToken: string;

  @Column({ nullable: true })
  refreshTokenHash: string;

  @CreateDateColumn()
  createdAt: Date;

  @OneToMany(() => Device, (device) => device.owner)
  devices: Device[];
}
