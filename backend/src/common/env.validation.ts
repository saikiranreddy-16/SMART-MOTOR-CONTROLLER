import { plainToInstance } from 'class-transformer';
import { IsEnum, IsNumber, IsString, IsOptional, validateSync } from 'class-validator';

export enum Environment {
  Development = 'development',
  Production = 'production',
  Test = 'test',
}

class EnvironmentVariables {
  @IsEnum(Environment)
  @IsOptional()
  NODE_ENV: Environment = Environment.Development;

  @IsNumber()
  PORT: number = 3000;

  @IsString()
  DATABASE_HOST: string;

  @IsNumber()
  DATABASE_PORT: number = 5432;

  @IsString()
  DATABASE_USERNAME: string;

  @IsString()
  DATABASE_PASSWORD: string;

  @IsString()
  DATABASE_NAME: string;

  @IsString()
  JWT_SECRET: string;

  @IsString()
  @IsOptional()
  JWT_EXPIRES_IN: string = '30d';

  @IsString()
  MQTT_HOST: string;

  @IsNumber()
  MQTT_PORT: number = 1883;

  @IsString()
  @IsOptional()
  MQTT_USERNAME?: string;

  @IsString()
  @IsOptional()
  MQTT_PASSWORD?: string;

  @IsString()
  REDIS_HOST: string;

  @IsNumber()
  REDIS_PORT: number = 6379;

  @IsString()
  OPEN_METEO_URL: string;
}

export function validate(config: Record<string, any>) {
  const validatedConfig = plainToInstance(
    EnvironmentVariables,
    {
      ...config,
      PORT: config.PORT ? parseInt(config.PORT, 10) : undefined,
      DATABASE_PORT: config.DATABASE_PORT ? parseInt(config.DATABASE_PORT, 10) : undefined,
      MQTT_PORT: config.MQTT_PORT ? parseInt(config.MQTT_PORT, 10) : undefined,
      REDIS_PORT: config.REDIS_PORT ? parseInt(config.REDIS_PORT, 10) : undefined,
    },
    { enableImplicitConversion: true },
  );

  const errors = validateSync(validatedConfig, { skipMissingProperties: false });

  if (errors.length > 0) {
    throw new Error(`Environment validation failed:\n${errors.toString()}`);
  }
  return validatedConfig;
}
