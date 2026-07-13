import { Controller, Post, Delete, Get, Body, Param, UseGuards, Inject } from '@nestjs/common';
import { JwtAuthGuard } from '../auth/jwt-auth.guard';
import { RolesGuard } from '../auth/roles.guard';
import { Roles } from '../auth/roles.decorator';
import { UserRole } from '../database/entities/user.entity';
import { DataSource } from 'typeorm';
import { Member } from '../database/entities/member.entity';
import { ApiTags, ApiOperation, ApiBearerAuth, ApiProperty } from '@nestjs/swagger';
import { IsString, IsNotEmpty, IsEnum } from 'class-validator';

class AddMemberDto {
  @ApiProperty({ example: '+919111222333' })
  @IsString()
  @IsNotEmpty()
  phoneNumber: string;

  @ApiProperty({ example: 'Kiran Kumar' })
  @IsString()
  @IsNotEmpty()
  name: string;

  @ApiProperty({ enum: UserRole, example: UserRole.MEMBER })
  @IsEnum(UserRole)
  role: UserRole;
}

@ApiTags('Device Members Whitelist')
@ApiBearerAuth()
@UseGuards(JwtAuthGuard, RolesGuard)
@Controller('devices/:deviceId/members')
export class MembersController {
  constructor(@Inject('DataSource') private readonly dataSource: DataSource) {}

  @Post()
  @Roles(UserRole.FARMER, UserRole.ADMIN)
  @ApiOperation({ summary: 'Whitelists a new phone number to command the motor starter' })
  async addMember(@Param('deviceId') deviceId: string, @Body() dto: AddMemberDto) {
    const member = new Member();
    member.deviceId = deviceId;
    member.name = dto.name;
    member.phoneNumber = dto.phoneNumber;
    member.role = dto.role || UserRole.MEMBER;

    return this.dataSource.getRepository(Member).save(member);
  }

  @Get()
  @ApiOperation({ summary: 'Lists all whitelisted members for a device' })
  async listMembers(@Param('deviceId') deviceId: string) {
    return this.dataSource.getRepository(Member).find({ where: { deviceId } });
  }

  @Delete(':phoneNumber')
  @Roles(UserRole.FARMER, UserRole.ADMIN)
  @ApiOperation({ summary: 'Removes a phone number from the starter whitelist' })
  async removeMember(@Param('deviceId') deviceId: string, @Param('phoneNumber') phoneNumber: string) {
    await this.dataSource.getRepository(Member).delete({ deviceId, phoneNumber });
    return { message: 'Member successfully removed.' };
  }
}
