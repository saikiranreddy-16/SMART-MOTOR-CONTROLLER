import { Entity, PrimaryGeneratedColumn, Column, CreateDateColumn } from 'typeorm';

@Entity('ota_updates')
export class OtaUpdate {
  @PrimaryGeneratedColumn('uuid')
  id: string;

  @Column({ unique: true })
  version: string;

  @Column()
  fileUrl: string;

  @Column({ default: true })
  isActive: boolean;

  @Column({ nullable: true })
  checksum: string; // MD5/SHA256 hash

  @CreateDateColumn()
  createdAt: Date;
}
