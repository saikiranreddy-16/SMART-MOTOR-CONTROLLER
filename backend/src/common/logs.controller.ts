import { Controller, Get, Param, UseGuards, Inject } from '@nestjs/common';
import { JwtAuthGuard } from '../auth/jwt-auth.guard';
import { DataSource } from 'typeorm';
import { DeviceLog } from '../database/entities/device-log.entity';
import { ApiTags, ApiOperation, ApiBearerAuth } from '@nestjs/swagger';

@ApiTags('Audit Logs')
@ApiBearerAuth()
@UseGuards(JwtAuthGuard)
@Controller('logs')
export class LogsController {
  constructor(@Inject('DataSource') private readonly dataSource: DataSource) {}

  @Get(':deviceId')
  @ApiOperation({ summary: 'Queries audit and safety trip event logs for a device' })
  async getDeviceLogs(@Param('deviceId') deviceId: string) {
    return this.dataSource.getRepository(DeviceLog).find({
      where: { deviceId },
      order: { timestamp: 'DESC' },
    });
  }
}
