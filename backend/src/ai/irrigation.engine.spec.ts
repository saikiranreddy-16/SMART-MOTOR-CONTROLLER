import { IrrigationEngine } from './irrigation.engine';

describe('IrrigationEngine', () => {
  let engine: IrrigationEngine;

  beforeEach(() => {
    engine = new IrrigationEngine();
  });

  it('should calculate soil moisture deficit correctly', () => {
    // Sandy Soil (Target: 45%)
    expect(engine.calculateWaterDeficit(30, 'Sandy')).toBe(15);
    expect(engine.calculateWaterDeficit(50, 'Sandy')).toBe(0);

    // Clay Soil (Target: 65%)
    expect(engine.calculateWaterDeficit(50, 'Clay')).toBe(15);
    expect(engine.calculateWaterDeficit(70, 'Clay')).toBe(0);
  });
});
