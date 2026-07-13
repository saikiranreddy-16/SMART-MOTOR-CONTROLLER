import { Test, TestingModule } from '@nestjs/testing';
import { MqttService } from './mqtt.service';
import { DeviceGateway } from './device.gateway';

jest.mock('ioredis', () => {
  return {
    default: jest.fn().mockImplementation(() => {
      return {
        get: jest.fn(),
        set: jest.fn(),
        del: jest.fn(),
      };
    }),
  };
});

describe('MqttService', () => {
  let service: MqttService;

  const mockDataSource = {};
  const mockDeviceGateway = {};

  beforeEach(async () => {
    const module: TestingModule = await Test.createTestingModule({
      providers: [
        MqttService,
        { provide: 'DataSource', useValue: mockDataSource },
        { provide: DeviceGateway, useValue: mockDeviceGateway },
      ],
    }).compile();

    service = module.get<MqttService>(MqttService);
  });

  it('should be defined', () => {
    expect(service).toBeDefined();
  });
});
