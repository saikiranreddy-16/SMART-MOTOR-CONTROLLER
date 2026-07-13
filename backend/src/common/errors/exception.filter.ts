import { ExceptionFilter, Catch, ArgumentsHost, HttpException, HttpStatus, Logger } from '@nestjs/common';
import { Request, Response } from 'express';
import { ErrorCode } from './error-codes';
import { BusinessException } from './error-response';

@Catch()
export class GlobalExceptionFilter implements ExceptionFilter {
  private readonly logger = new Logger('GlobalExceptionFilter');

  catch(exception: any, host: ArgumentsHost) {
    const ctx = host.switchToHttp();
    const response = ctx.getResponse<Response>();
    const request = ctx.getRequest<Request>();
    const requestId = request.headers['x-request-id'] || 'N/A';

    let status = HttpStatus.INTERNAL_SERVER_ERROR;
    let errorCode = ErrorCode.INTERNAL_SERVER_ERROR;
    let message = 'Internal Server Error';

    if (exception instanceof BusinessException) {
      status = exception.getStatus();
      errorCode = exception.errorCode;
      message = exception.message;
    } else if (exception instanceof HttpException) {
      status = exception.getStatus();
      const resBody = exception.getResponse() as any;
      message = typeof resBody === 'string' ? resBody : resBody.message || exception.message;

      // Map common auth exceptions to E010
      if (status === HttpStatus.UNAUTHORIZED || status === HttpStatus.FORBIDDEN) {
        errorCode = ErrorCode.AUTHENTICATION_FAILED;
      }
    } else if (exception instanceof Error) {
      message = exception.message;
      // Map TypeORM errors to E009
      if (exception.name?.includes('QueryFailedError') || exception.message?.includes('database')) {
        errorCode = ErrorCode.DATABASE_ERROR;
      }
    }

    this.logger.error(
      `[Req ID: ${requestId}] Exception on ${request.method} ${request.url} | Status: ${status} | ErrorCode: ${errorCode} | Msg: ${message}`,
      exception.stack,
    );

    response.status(status).json({
      statusCode: status,
      errorCode,
      message: Array.isArray(message) ? message[0] : message,
      timestamp: new Date().toISOString(),
      path: request.url,
      requestId,
    });
  }
}
