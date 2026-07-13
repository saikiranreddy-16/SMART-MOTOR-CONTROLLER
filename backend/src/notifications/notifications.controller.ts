import { Controller, Get, Post, Param, UseGuards, Inject, Request } from '@nestjs/common';
import { JwtAuthGuard } from '../auth/jwt-auth.guard';
import { DataSource } from 'typeorm';
import { Notification } from '../database/entities/notification.entity';
import { ApiTags, ApiOperation, ApiBearerAuth } from '@nestjs/swagger';

@ApiTags('Notifications')
@ApiBearerAuth()
@UseGuards(JwtAuthGuard)
@Controller('notifications')
export class NotificationsController {
  constructor(@Inject('DataSource') private readonly dataSource: DataSource) {}

  @Get()
  @ApiOperation({ summary: 'Retrieves all notifications for the authenticated user' })
  async getNotifications(@Request() req: any) {
    return this.dataSource.getRepository(Notification).find({
      where: { userId: req.user.id },
      order: { createdAt: 'DESC' },
    });
  }

  @Post(':id/read')
  @ApiOperation({ summary: 'Marks a notification as read' })
  async markAsRead(@Param('id') id: string) {
    await this.dataSource.getRepository(Notification).update(id, { isRead: true });
    return { success: true };
  }
}
