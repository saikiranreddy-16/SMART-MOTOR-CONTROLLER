# Database Schema Guide (Version 1.0 Commercial Edition)

This guide documents the PostgreSQL database tables and relationships for the Version 1.0 Enterprise Backend.

---

## 1. Entity Relationship Diagram (PostgreSQL)

```mermaid
erDiagram
    users ||--o{ devices : "owns"
    devices ||--o{ device_whitelist : "whitelists"
    devices ||--o{ telemetry_logs : "records"
    devices ||--o{ audit_logs : "audits"
    devices ||--o{ irrigation_recommendations : "calculates"

    users {
        int id PK
        string uid UNIQUE
        string name
        string email UNIQUE
        string phone_number UNIQUE
        enum role "Owner, Family, Worker"
        string fcm_token
        timestamp created_at
    }

    devices {
        string id PK "ESP32 MAC ID"
        string name
        int owner_id FK
        string sim_phone_number UNIQUE
        string sim_imsi UNIQUE
        enum pump_type "AC_SINGLE_PHASE, AC_THREE_PHASE, SOLAR_VFD"
        int active_language
        jsonb safety_thresholds
        timestamp registered_at
    }

    device_whitelist {
        int id PK
        string device_id FK
        string phone_number
        enum role "Owner, Family, Worker"
    }

    telemetry_logs {
        bigint id PK
        string device_id FK
        timestamp timestamp
        real voltage_r
        real voltage_y
        real voltage_b
        real current_r
        real current_y
        real current_b
        real water_level
        real soil_moisture
        real casing_temp
        boolean power_available
    }
```

---

## 2. PostgreSQL Table Specifications

### Table: `users`
Stores farmer account information.

* `id` (SERIAL, Primary Key): Auto-incrementing identifier.
* `uid` (VARCHAR, Unique): Firebase / external auth provider mapping token.
* `phone_number` (VARCHAR, Unique, Indexed): Primary mobile number (used for SMS OTP verify).
* `role` (ENUM): User privileges - `Owner`, `Family`, or `Worker`.

### Table: `devices`
Maintains hardware profiles.

* `id` (VARCHAR, Primary Key): Unique MAC address of the ESP32.
* `pump_type` (ENUM): Operating profile - `AC_SINGLE_PHASE`, `AC_THREE_PHASE`, or `SOLAR_VFD`.
* `active_language` (INTEGER): PERSISTED language selection index - `1` (English), `2` (Telugu), `3` (Hindi).
* `safety_thresholds` (JSONB): Dynamic configuration maps containing voltage and current limits.

### Table: `telemetry_logs`
Time-series table capturing electrical and moisture parameters.

* `device_id` (VARCHAR, Foreign Key): Linked controller card.
* `timestamp` (TIMESTAMP WITH TIME ZONE, Indexed): Sample epoch.
* `voltage_r`, `voltage_y`, `voltage_b` (REAL): Phase Voltages.
* `water_level` (REAL): Hydrostatic depth level percentage.
* `power_available` (BOOLEAN): Grid presence marker.
