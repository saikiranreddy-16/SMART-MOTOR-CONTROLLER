-- PostgreSQL Schema definition for Version 1.0 Commercial Edition

-- Enums
CREATE TYPE user_role AS ENUM ('Owner', 'Family', 'Worker');
CREATE TYPE pump_type_enum AS ENUM ('AC_SINGLE_PHASE', 'AC_THREE_PHASE', 'SOLAR_VFD');
CREATE TYPE log_category AS ENUM ('FAULT', 'USER_ACTION', 'SECURITY');

-- 1. Users Table
CREATE TABLE users (
    id SERIAL PRIMARY KEY,
    uid VARCHAR(128) UNIQUE NOT NULL, -- Firebase UID or auth server identifier
    name VARCHAR(100) NOT NULL,
    email VARCHAR(100) UNIQUE,
    phone_number VARCHAR(20) UNIQUE NOT NULL,
    role user_role DEFAULT 'Worker',
    fcm_token VARCHAR(255),
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- 2. Devices Table
CREATE TABLE devices (
    id VARCHAR(64) PRIMARY KEY, -- ESP32 chip MAC ID
    name VARCHAR(100) NOT NULL,
    owner_id INTEGER REFERENCES users(id) ON DELETE SET NULL,
    sim_phone_number VARCHAR(20) UNIQUE,
    sim_imsi VARCHAR(30) UNIQUE,
    pump_type pump_type_enum DEFAULT 'AC_THREE_PHASE',
    active_language INTEGER DEFAULT 1, -- 1 = ENG, 2 = TEL, 3 = HIN
    safety_thresholds JSONB NOT NULL DEFAULT '{
        "under_voltage_limit": 180.0,
        "over_voltage_limit": 260.0,
        "imbalance_percent": 15.0,
        "dry_run_current_multiplier": 0.60,
        "dry_run_cooldown_minutes": 45,
        "overcurrent_nominal_amps": 15.0
    }',
    registered_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- 3. Device Whitelist Mapping (For Family and Worker numbers)
CREATE TABLE device_whitelist (
    id SERIAL PRIMARY KEY,
    device_id VARCHAR(64) REFERENCES devices(id) ON DELETE CASCADE,
    phone_number VARCHAR(20) NOT NULL,
    role user_role NOT NULL DEFAULT 'Worker',
    UNIQUE (device_id, phone_number)
);

-- 4. Telemetry History (Time-series log table)
CREATE TABLE telemetry_logs (
    id BIGSERIAL PRIMARY KEY,
    device_id VARCHAR(64) REFERENCES devices(id) ON DELETE CASCADE NOT NULL,
    timestamp TIMESTAMP WITH TIME ZONE NOT NULL,
    voltage_r REAL,
    voltage_y REAL,
    voltage_b REAL,
    current_r REAL,
    current_y REAL,
    current_b REAL,
    water_level REAL,
    soil_moisture REAL,
    casing_temp REAL,
    power_available BOOLEAN NOT NULL
);

-- Indexes for fast graphing queries
CREATE INDEX idx_telemetry_device_timestamp ON telemetry_logs (device_id, timestamp DESC);

-- 5. Audit logs
CREATE TABLE audit_logs (
    id SERIAL PRIMARY KEY,
    device_id VARCHAR(64) REFERENCES devices(id) ON DELETE CASCADE,
    timestamp TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,
    category log_category NOT NULL,
    event_name VARCHAR(100) NOT NULL, -- 'DRY_RUN', 'MOTOR_START', 'TAMPER'
    description TEXT,
    triggered_by VARCHAR(128) NOT NULL -- 'SYSTEM' or User UID
);

-- 6. AI Recommendations
CREATE TABLE irrigation_recommendations (
    id SERIAL PRIMARY KEY,
    device_id VARCHAR(64) REFERENCES devices(id) ON DELETE CASCADE,
    timestamp TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,
    recommended_duration_minutes INTEGER NOT NULL,
    rain_probability REAL,
    soil_moisture REAL,
    reason TEXT,
    applied BOOLEAN DEFAULT FALSE
);
