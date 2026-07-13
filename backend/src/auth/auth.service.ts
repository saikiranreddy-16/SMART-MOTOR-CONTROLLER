import { Injectable, UnauthorizedException, Inject, BadRequestException } from '@nestjs/common';
import { JwtService } from '@nestjs/jwt';
import { DataSource } from 'typeorm';
import { User } from '../database/entities/user.entity';
import { UserRole } from '../database/entities/user-role.enum';
import Redis from 'ioredis';
import * as bcrypt from 'bcrypt';

@Injectable()
export class AuthService {
  private redis: Redis;

  constructor(
    @Inject('DataSource') private readonly dataSource: DataSource,
    private readonly jwtService: JwtService,
  ) {
    this.redis = new Redis({
      host: process.env.REDIS_HOST || 'localhost',
      port: parseInt(process.env.REDIS_PORT) || 6379,
    });
  }

  // 1. Generate a 6-digit numeric OTP and cache it in Redis
  async generateOtp(phoneNumber: string): Promise<string> {
    const otp = Math.floor(100000 + Math.random() * 900000).toString();
    await this.redis.set(`otp:${phoneNumber}`, otp, 'EX', 300); // 5 min TTL
    console.log(`[SMS OTP Dispatcher] Send OTP: ${otp} to phone: ${phoneNumber}`);
    return otp;
  }

  // 2. Register a new user
  async register(name: string, phoneNumber: string, password: string, role: UserRole): Promise<User> {
    const userRepo = this.dataSource.getRepository(User);
    const existing = await userRepo.findOne({ where: { phoneNumber } });
    if (existing) {
      throw new BadRequestException('User with this phone number already exists.');
    }

    const passwordHash = await bcrypt.hash(password, 10);
    const user = new User();
    user.name = name;
    user.phoneNumber = phoneNumber;
    user.passwordHash = passwordHash;
    user.role = role;

    return userRepo.save(user);
  }

  // 3. Login with password and get access/refresh tokens
  async loginWithPassword(phoneNumber: string, password: string) {
    const userRepo = this.dataSource.getRepository(User);
    const user = await userRepo.findOne({ where: { phoneNumber } });
    if (!user) {
      throw new UnauthorizedException('Invalid credentials.');
    }

    const isValid = await bcrypt.compare(password, user.passwordHash);
    if (!isValid) {
      throw new UnauthorizedException('Invalid credentials.');
    }

    return this.generateTokens(user);
  }

  // 4. Verify OTP and authenticate
  async verifyOtpAndLogin(phoneNumber: string, submittedOtp: string) {
    const cachedOtp = await this.redis.get(`otp:${phoneNumber}`);
    if (!cachedOtp || cachedOtp !== submittedOtp) {
      throw new UnauthorizedException('Invalid or expired OTP code.');
    }

    await this.redis.del(`otp:${phoneNumber}`);

    const userRepo = this.dataSource.getRepository(User);
    let user = await userRepo.findOne({ where: { phoneNumber } });
    if (!user) {
      // Auto-register as Member
      user = new User();
      user.name = 'Farmer Member';
      user.phoneNumber = phoneNumber;
      user.passwordHash = await bcrypt.hash(Math.random().toString(36), 10);
      user.role = UserRole.MEMBER;
      user = await userRepo.save(user);
    }

    return this.generateTokens(user);
  }

  // 5. Refresh token exchange
  async refreshTokens(refreshToken: string) {
    try {
      const payload = this.jwtService.verify(refreshToken);
      const userRepo = this.dataSource.getRepository(User);
      const user = await userRepo.findOne({ where: { id: payload.sub } });
      if (!user || !user.refreshTokenHash) {
        throw new UnauthorizedException('Invalid refresh token.');
      }

      const isMatch = await bcrypt.compare(refreshToken, user.refreshTokenHash);
      if (!isMatch) {
        throw new UnauthorizedException('Invalid refresh token.');
      }

      return this.generateTokens(user);
    } catch {
      throw new UnauthorizedException('Expired or invalid refresh token.');
    }
  }

  private async generateTokens(user: User) {
    const payload = { sub: user.id, phone: user.phoneNumber, role: user.role };

    const accessToken = this.jwtService.sign(payload, { expiresIn: '15m' });
    const refreshToken = this.jwtService.sign(payload, { expiresIn: '30d' });

    // Store hashed refresh token in DB
    const userRepo = this.dataSource.getRepository(User);
    user.refreshTokenHash = await bcrypt.hash(refreshToken, 10);
    await userRepo.save(user);

    return {
      accessToken,
      refreshToken,
      user: {
        id: user.id,
        name: user.name,
        phoneNumber: user.phoneNumber,
        role: user.role,
      },
    };
  }
}
