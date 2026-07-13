import { Injectable } from '@nestjs/common';

@Injectable()
export class PredictionEngine {
  // Evaluates phase current distributions to predict mechanical bearing decay
  predictMotorHealth(
    currentR: number,
    currentY: number,
    currentB: number,
  ): { healthScore: number; recommendation: string } {
    const avg = (currentR + currentY + currentB) / 3.0;
    if (avg < 0.5) {
      return { healthScore: 100, recommendation: 'Motor Standby' };
    }

    const diffR = Math.abs(currentR - avg) / avg;
    const diffY = Math.abs(currentY - avg) / avg;
    const diffB = Math.abs(currentB - avg) / avg;
    const maxImbalance = Math.max(diffR, diffY, diffB);

    if (maxImbalance > 0.2) {
      return {
        healthScore: 55,
        recommendation: 'Bearing Wear Alert: High Current Imbalance. Schedule winding check.',
      };
    }

    if (maxImbalance > 0.12) {
      return {
        healthScore: 78,
        recommendation: 'Winding Warning: Moderate Phase Asymmetric load.',
      };
    }

    return { healthScore: 98, recommendation: 'Motor Winding & Bearings Healthy' };
  }
}
