import { Injectable } from '@nestjs/common';

@Injectable()
export class IrrigationEngine {
  // Computes soil moisture deficit percentage
  calculateWaterDeficit(currentMoisture: number, soilType: string): number {
    // Target moisture threshold percent by soil characteristic
    const targetMoisture = soilType === 'Clay' ? 65 : soilType === 'Sandy' ? 45 : 55;

    if (currentMoisture >= targetMoisture) return 0;
    return targetMoisture - currentMoisture;
  }
}
