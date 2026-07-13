import { Injectable, NestMiddleware, Logger } from '@nestjs/common';
import { Request, Response, NextFunction } from 'express';
import { randomUUID } from 'crypto';

@Injectable()
export class RequestIdMiddleware implements NestMiddleware {
  private readonly logger = new Logger('HTTP');

  use(req: Request, res: Response, next: NextFunction) {
    const requestId = (req.headers['x-request-id'] as string) || randomUUID();

    // Set headers
    req.headers['x-request-id'] = requestId;
    res.setHeader('x-request-id', requestId);

    const { method, originalUrl, ip } = req;
    const userAgent = req.get('user-agent') || '';
    const startTime = Date.now();

    res.on('finish', () => {
      const { statusCode } = res;
      const duration = Date.now() - startTime;

      this.logger.log(
        `[Req ID: ${requestId}] ${method} ${originalUrl} ${statusCode} - ${userAgent} ${ip} - ${duration}ms`,
      );
    });

    next();
  }
}
