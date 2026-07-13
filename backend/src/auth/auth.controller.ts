import { Controller, Post, Body, HttpCode, HttpStatus } from '@nestjs/common';
import { AuthService } from './auth.service';
import { ApiTags, ApiOperation, ApiResponse, ApiProperty } from '@nestjs/swagger';
import { UserRole } from '../database/entities/user-role.enum';
import { IsString, IsNotEmpty, IsEnum } from 'class-validator';

class SignupDto {
  @ApiProperty({ example: 'Rajesh Kumar', description: 'Name of the user' })
  @IsString()
  @IsNotEmpty()
  name: string;

  @ApiProperty({ example: '+919876543210', description: 'Phone number of the user' })
  @IsString()
  @IsNotEmpty()
  phoneNumber: string;

  @ApiProperty({ example: 'password123', description: 'Plain text password of the user' })
  @IsString()
  @IsNotEmpty()
  passwordHash: string; // password

  @ApiProperty({ enum: UserRole, example: UserRole.FARMER, description: 'Designated user role' })
  @IsEnum(UserRole)
  role: UserRole;
}

class LoginDto {
  @ApiProperty({ example: '+919876543210' })
  @IsString()
  @IsNotEmpty()
  phoneNumber: string;

  @ApiProperty({ example: 'password123' })
  @IsString()
  @IsNotEmpty()
  passwordHash: string; // password
}

class OtpGenerateDto {
  @ApiProperty({ example: '+919876543210' })
  @IsString()
  @IsNotEmpty()
  phoneNumber: string;
}

class OtpVerifyDto {
  @ApiProperty({ example: '+919876543210' })
  @IsString()
  @IsNotEmpty()
  phoneNumber: string;

  @ApiProperty({ example: '123456' })
  @IsString()
  @IsNotEmpty()
  otp: string;
}

class RefreshDto {
  @ApiProperty({ example: 'some-jwt-refresh-token' })
  @IsString()
  @IsNotEmpty()
  refreshToken: string;
}

@ApiTags('Authentication')
@Controller('auth')
export class AuthController {
  constructor(private readonly authService: AuthService) {}

  @Post('signup')
  @ApiOperation({ summary: 'Registers a new user (Farmer, Member, Installer)' })
  @ApiResponse({ status: 201, description: 'User successfully created' })
  async signup(@Body() signupDto: SignupDto) {
    return this.authService.register(signupDto.name, signupDto.phoneNumber, signupDto.passwordHash, signupDto.role);
  }

  @Post('login')
  @HttpCode(HttpStatus.OK)
  @ApiOperation({ summary: 'Authenticates with phone number and password' })
  @ApiResponse({ status: 200, description: 'Tokens issued successfully' })
  async login(@Body() loginDto: LoginDto) {
    return this.authService.loginWithPassword(loginDto.phoneNumber, loginDto.passwordHash);
  }

  @Post('otp/generate')
  @HttpCode(HttpStatus.OK)
  @ApiOperation({ summary: 'Generates a login OTP' })
  async generateOtp(@Body() otpDto: OtpGenerateDto) {
    const otp = await this.authService.generateOtp(otpDto.phoneNumber);
    return { message: 'OTP dispatched successfully.', otp }; // Return for testing
  }

  @Post('otp/verify')
  @HttpCode(HttpStatus.OK)
  @ApiOperation({ summary: 'Verifies login OTP' })
  async verifyOtp(@Body() verifyDto: OtpVerifyDto) {
    return this.authService.verifyOtpAndLogin(verifyDto.phoneNumber, verifyDto.otp);
  }

  @Post('refresh')
  @HttpCode(HttpStatus.OK)
  @ApiOperation({ summary: 'Refreshes expired access tokens' })
  async refresh(@Body() refreshDto: RefreshDto) {
    return this.authService.refreshTokens(refreshDto.refreshToken);
  }
}
