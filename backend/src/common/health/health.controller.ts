import { Controller, Get, Inject } from '@nestjs/common';
import { DataSource } from 'typeorm';
import { ApiTags, ApiOperation, ApiResponse } from '@nestjs/swagger';

@ApiTags('Health')
@Controller('health')
export class HealthController {
  constructor(@Inject('DataSource') private readonly dataSource: DataSource) {}

  @Get()
  @ApiOperation({ summary: 'Checks system and database health status' })
  @ApiResponse({ status: 200, description: 'System is healthy' })
  @ApiResponse({ status: 500, description: 'Database connection failed' })
  async getHealth() {
    let dbStatus = 'DISCONNECTED';
    try {
      const isConnected = this.dataSource.isInitialized;
      if (isConnected) {
        // Run simple query to check connection
        await this.dataSource.query('SELECT 1');
        dbStatus = 'CONNECTED';
      }
    } catch (err) {
      dbStatus = `FAILED: ${err.message}`;
    }

    const isHealthy = dbStatus === 'CONNECTED';
    return {
      status: isHealthy ? 'UP' : 'DOWN',
      database: dbStatus,
      timestamp: new Date().toISOString(),
    };
  }
}
