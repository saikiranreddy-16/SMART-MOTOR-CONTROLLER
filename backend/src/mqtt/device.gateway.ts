import { WebSocketGateway, WebSocketServer, OnGatewayConnection, OnGatewayDisconnect } from '@nestjs/websockets';
import { Server } from 'ws';
import { Logger } from '@nestjs/common';

@WebSocketGateway({
  path: '/ws/devices',
})
export class DeviceGateway implements OnGatewayConnection, OnGatewayDisconnect {
  private readonly logger = new Logger('DeviceGateway');

  @WebSocketServer()
  server: Server;

  handleConnection(client: any) {
    this.logger.log('Client connected to WebSocket Gateway');
  }

  handleDisconnect(client: any) {
    this.logger.log('Client disconnected from WebSocket Gateway');
  }

  // Emits status/telemetry updates to all connected apps
  broadcastDeviceUpdate(deviceId: string, data: any) {
    if (!this.server || !this.server.clients) {
      return;
    }
    const payload = JSON.stringify({ deviceId, ...data });
    this.server.clients.forEach((client) => {
      if (client.readyState === 1) {
        // OPEN
        client.send(payload);
      }
    });
  }
}
