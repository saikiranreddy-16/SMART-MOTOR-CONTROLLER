import { Controller, Post, Get, Body, Param, UseGuards, Inject, Query, HttpStatus } from '@nestjs/common';
import { JwtAuthGuard } from '../auth/jwt-auth.guard';
import { RolesGuard } from '../auth/roles.guard';
import { Roles } from '../auth/roles.decorator';
import { UserRole } from '../database/entities/user.entity';
import { MqttService } from '../mqtt/mqtt.service';
import { DataSource } from 'typeorm';
import { TelemetryLog } from '../database/entities/telemetry-log.entity';
import { PumpHistory } from '../database/entities/pump-history.entity';
import { Device, PumpTypeEnum } from '../database/entities/device.entity';
import { Motor } from '../database/entities/motor.entity';
import { WaterLevel } from '../database/entities/water-level.entity';
import { ApiTags, ApiOperation, ApiBearerAuth, ApiResponse, ApiProperty } from '@nestjs/swagger';
import { BusinessException } from '../common/errors/error-response';
import { ErrorCode } from '../common/errors/error-codes';
import { IsString, IsNotEmpty, IsEnum } from 'class-validator';

class CommandDto {
  @ApiProperty({ enum: ['START', 'STOP'], example: 'START' })
  @IsEnum(['START', 'STOP'])
  @IsNotEmpty()
  action: 'START' | 'STOP';

  @ApiProperty({ example: 'APP', description: 'Sender of trigger (e.g. APP, SMS, BT)' })
  @IsString()
  @IsNotEmpty()
  triggeredBy: string;
}

class RegisterDeviceDto {
  @ApiProperty({ example: 'ESP32_A83C10', description: 'Device MAC address ID' })
  @IsString()
  @IsNotEmpty()
  deviceId: string;

  @ApiProperty({ example: 'Borewell Pump 1' })
  @IsString()
  @IsNotEmpty()
  name: string;

  @ApiProperty({ example: '+919988776655' })
  @IsString()
  @IsNotEmpty()
  simPhoneNumber: string;

  @ApiProperty({ enum: PumpTypeEnum, example: PumpTypeEnum.AC_THREE })
  @IsEnum(PumpTypeEnum)
  pumpType: PumpTypeEnum;
}

@ApiTags('Motor Control & Devices')
@ApiBearerAuth()
@UseGuards(JwtAuthGuard, RolesGuard)
@Controller('devices')
export class MotorController {
  constructor(
    private readonly mqttService: MqttService,
    @Inject('DataSource') private readonly dataSource: DataSource,
  ) {}

  @Post()
  @Roles(UserRole.FARMER, UserRole.ADMIN)
  @ApiOperation({ summary: 'Registers a new motor controller node' })
  async registerDevice(@Body() dto: RegisterDeviceDto) {
    const deviceRepo = this.dataSource.getRepository(Device);

    let device = await deviceRepo.findOne({ where: { id: dto.deviceId } });
    if (device) {
      throw new BusinessException('Device already registered.', ErrorCode.DATABASE_ERROR);
    }

    device = new Device();
    device.id = dto.deviceId;
    device.name = dto.name;
    device.simPhoneNumber = dto.simPhoneNumber;
    device.pumpType = dto.pumpType;

    const savedDevice = await deviceRepo.save(device);

    // Initialize Motor associated entity
    const motor = new Motor();
    motor.deviceId = device.id;
    await this.dataSource.getRepository(Motor).save(motor);

    // Initialize WaterLevel associated entity
    const waterLevel = new WaterLevel();
    waterLevel.deviceId = device.id;
    await this.dataSource.getRepository(WaterLevel).save(waterLevel);

    return savedDevice;
  }

  @Get()
  @ApiOperation({ summary: 'Lists all registered motor nodes' })
  async listDevices() {
    return this.dataSource.getRepository(Device).find({ relations: ['owner'] });
  }

  @Post(':id/command')
  @Roles(UserRole.FARMER, UserRole.ADMIN, UserRole.MEMBER)
  @ApiOperation({ summary: 'Dispatches start/stop motor signals' })
  async sendCommand(@Param('id') deviceId: string, @Body() dto: CommandDto) {
    const device = await this.dataSource.getRepository(Device).findOne({ where: { id: deviceId } });
    if (!device) {
      throw new BusinessException('Device not found.', ErrorCode.DATABASE_ERROR, HttpStatus.NOT_FOUND);
    }

    // Publish to MQTT
    this.mqttService.publishCommand(deviceId, {
      command: dto.action,
      triggeredBy: dto.triggeredBy || 'APP',
      timestamp: Math.floor(Date.now() / 1000),
    });

    // Write command to history log
    const history = new PumpHistory();
    history.deviceId = deviceId;
    history.startedAt = new Date();
    history.triggeredBy = dto.triggeredBy || 'APP';
    history.status = dto.action === 'START' ? 'STARTING' : 'STOPPING';
    await this.dataSource.getRepository(PumpHistory).save(history);

    return { message: `Command ${dto.action} successfully dispatched.` };
  }

  @Get(':id/telemetry')
  @ApiOperation({ summary: 'Queries telemetry logs for a given device' })
  async getTelemetry(@Param('id') deviceId: string, @Query('limit') limit = 20) {
    return this.dataSource.getRepository(TelemetryLog).find({
      where: { deviceId },
      order: { timestamp: 'DESC' },
      take: limit,
    });
  }

  @Get(':id/history')
  @ApiOperation({ summary: 'Queries pump operation history logs' })
  async getHistory(@Param('id') deviceId: string) {
    return this.dataSource.getRepository(PumpHistory).find({
      where: { deviceId },
      order: { startedAt: 'DESC' },
    });
  }
}
