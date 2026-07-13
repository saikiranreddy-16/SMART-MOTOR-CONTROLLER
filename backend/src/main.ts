import 'dotenv/config';
import { NestFactory } from '@nestjs/core';
import { AppModule } from './app.module';
import { ValidationPipe, VersioningType } from '@nestjs/common';
import { SwaggerModule, DocumentBuilder } from '@nestjs/swagger';
import { GlobalExceptionFilter } from './common/errors/exception.filter';
import { WsAdapter } from '@nestjs/platform-ws';
import helmet from 'helmet';
import rateLimit from 'express-rate-limit';

async function bootstrap() {
  const app = await NestFactory.create(AppModule);

  // Use ws for WebSockets
  app.useWebSocketAdapter(new WsAdapter(app));

  // 1. Enforce security headers
  app.use(helmet());

  // 2. Enable CORS
  app.enableCors({
    origin: '*',
    methods: 'GET,HEAD,PUT,PATCH,POST,DELETE,OPTIONS',
    credentials: true,
  });

  // 3. Request Rate Limiting (100 requests per 15 minutes)
  app.use(
    rateLimit({
      windowMs: 15 * 60 * 1000,
      max: 100,
      message: 'Too many requests from this IP, please try again after 15 minutes',
      headers: true,
    }),
  );

  // 4. Global API prefixing & Versioning
  app.setGlobalPrefix('api');
  app.enableVersioning({
    type: VersioningType.URI,
    defaultVersion: '1', // Results in: /api/v1/...
  });

  // 5. Global Pipes & Filters
  app.useGlobalPipes(
    new ValidationPipe({
      whitelist: true,
      transform: true,
      forbidNonWhitelisted: true,
    }),
  );
  app.useGlobalFilters(new GlobalExceptionFilter());

  // 6. Integrate Swagger Docs
  const config = new DocumentBuilder()
    .setTitle('Smart Agricultural Motor Controller API')
    .setDescription('Commercial Industrial V1.0 API Documentation')
    .setVersion('1.0')
    .addBearerAuth()
    .build();

  const document = SwaggerModule.createDocument(app, config);
  SwaggerModule.setup('api', app, document);

  const port = process.env.PORT || 3000;
  await app.listen(port);
  console.log(`=================================================`);
  console.log(`Smart Farm NestJS Engine running on port: ${port}`);
  console.log(`Swagger docs exposed at http://localhost:${port}/api`);
  console.log(`=================================================`);
}
bootstrap();
