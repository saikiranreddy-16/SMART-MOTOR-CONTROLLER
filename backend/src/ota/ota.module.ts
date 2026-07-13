import { Module } from '@nestjs/common';
import { OtaController } from './ota.controller';

@Module({
  controllers: [OtaController],
})
export class OtaModule {}
