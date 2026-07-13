import { Test, TestingModule } from '@nestjs/testing';
import { AuthService } from './auth.service';
import { JwtService } from '@nestjs/jwt';
import { DataSource } from 'typeorm';

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

describe('AuthService', () => {
  let service: AuthService;
  let jwtService: JwtService;

  const mockDataSource = {
    getRepository: jest.fn().mockReturnValue({
      findOne: jest.fn(),
      save: jest.fn(),
    }),
  };

  const mockJwtService = {
    sign: jest.fn().mockReturnValue('mock-token'),
    verify: jest.fn().mockReturnValue({ sub: 'user-id' }),
  };

  beforeEach(async () => {
    const module: TestingModule = await Test.createTestingModule({
      providers: [
        AuthService,
        { provide: 'DataSource', useValue: mockDataSource },
        { provide: JwtService, useValue: mockJwtService },
      ],
    }).compile();

    service = module.get<AuthService>(AuthService);
    jwtService = module.get<JwtService>(JwtService);
  });

  it('should be defined', () => {
    expect(service).toBeDefined();
  });
});
