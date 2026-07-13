import { Controller, Post, Get, Body, UseGuards, Inject, HttpStatus } from '@nestjs/common';
import { JwtAuthGuard } from '../auth/jwt-auth.guard';
import { RolesGuard } from '../auth/roles.guard';
import { Roles } from '../auth/roles.decorator';
import { UserRole } from '../database/entities/user.entity';
import { DataSource } from 'typeorm';
import { OtaUpdate } from '../database/entities/ota-update.entity';
import { ApiTags, ApiOperation, ApiBearerAuth, ApiResponse, ApiProperty } from '@nestjs/swagger';
import { BusinessException } from '../common/errors/error-response';
import { ErrorCode } from '../common/errors/error-codes';
import { IsString, IsNotEmpty } from 'class-validator';

class ReleaseOtaDto {
  @ApiProperty({ example: 'v1.0.1' })
  @IsString()
  @IsNotEmpty()
  version: string;

  @ApiProperty({ example: 'https://storage.googleapis.com/ota-bins/v1.0.1.bin' })
  @IsString()
  @IsNotEmpty()
  fileUrl: string;

  @ApiProperty({ example: 'd41d8cd98f00b204e9800998ecf8427e' })
  @IsString()
  @IsNotEmpty()
  checksum: string;
}

@ApiTags('OTA Firmware Updates')
@ApiBearerAuth()
@UseGuards(JwtAuthGuard, RolesGuard)
@Controller('ota')
export class OtaController {
  constructor(@Inject('DataSource') private readonly dataSource: DataSource) {}

  @Post('release')
  @Roles(UserRole.ADMIN)
  @ApiOperation({ summary: 'Releases a new OTA update binary' })
  async releaseUpdate(@Body() dto: ReleaseOtaDto) {
    const otaRepo = this.dataSource.getRepository(OtaUpdate);
    const existing = await otaRepo.findOne({ where: { version: dto.version } });
    if (existing) {
      throw new BusinessException('Firmware version already released.', ErrorCode.DATABASE_ERROR);
    }

    // Set other versions inactive
    await otaRepo.update({}, { isActive: false });

    const update = new OtaUpdate();
    update.version = dto.version;
    update.fileUrl = dto.fileUrl;
    update.checksum = dto.checksum;
    update.isActive = true;

    return otaRepo.save(update);
  }

  @Get('latest')
  @Roles(UserRole.ADMIN, UserRole.INSTALLER, UserRole.FARMER)
  @ApiOperation({ summary: 'Queries the latest active firmware version details' })
  async getLatestVersion() {
    const ota = await this.dataSource.getRepository(OtaUpdate).findOne({ where: { isActive: true } });
    if (!ota) {
      throw new BusinessException('No active firmware updates found.', ErrorCode.OTA_FAILURE, HttpStatus.NOT_FOUND);
    }
    return ota;
  }
}
