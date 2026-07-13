# API & Payload Specifications (Version 1.0 Commercial Edition)

This document specifies the protocols, topics, REST endpoints, and SMS commands for the Version 1.0 Commercial Edition Smart Agricultural Motor Controller.

---

## 1. REST API Specification (NestJS Enterprise Server)

The REST API manages user login, authentication, and device management.

* **Base URL**: `https://api.smartfarm-controller.in/v1`
* **Authentication**: JWT token sent via `Authorization: Bearer <TOKEN>` header.

### 1.1 Request OTP Login Code
* **Endpoint**: `POST /auth/otp/request`
* **Request**:
```json
{
  "phone_number": "+919876543210"
}
```
* **Response (200 OK)**:
```json
{
  "status": "SUCCESS",
  "message": "OTP generated and dispatched via SMS gateway."
}
```

### 1.2 Verify OTP and Generate JWT Token
* **Endpoint**: `POST /auth/otp/verify`
* **Request**:
```json
{
  "phone_number": "+919876543210",
  "otp": "482039"
}
```
* **Response (200 OK)**:
```json
{
  "status": "SUCCESS",
  "token": "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9...",
  "user": {
    "name": "Rajesh Kumar",
    "role": "Owner"
  }
}
```

### 1.3 Register Device
* **Endpoint**: `POST /devices`
* **Request**:
```json
{
  "device_id": "ESP32_A83C10",
  "name": "Main Farm Borewell",
  "sim_phone_number": "+919988776655",
  "pump_type": "SOLAR_VFD"
}
```
* **Response (201 Created)**:
```json
{
  "status": "REGISTERED",
  "device_id": "ESP32_A83C10"
}
```

---

## 2. MQTT Telemetry Topic Specifications

Telemetry JSON payload updates include the active language, pump type, and paired remote ID.

#### Topic: `device/{device_id}/telemetry` (QoS 0)
```json
{
  "timestamp": 1782390150,
  "state": "RUNNING",
  "voltage": {
    "R": 0.0,
    "Y": 0.0,
    "B": 0.0,
    "balance": 0.0
  },
  "current": {
    "R": 12.4,
    "Y": 0.0,
    "B": 0.0
  },
  "sensors": {
    "water_level": 82.5,
    "soil_moisture": 42.0,
    "casing_temp": 45.2
  },
  "config": {
    "active_language": 1,
    "pump_type": 3,
    "paired_remote_id": 1419995410
  },
  "power_available": false
}
```

---

## 3. SMS Commands (Offline Fallback)

All SMS commands must be whitelisted. Response prefixes are formatted as `SF ` to assert validation.

### 3.1 Language Settings Configuration
* **Command**: `LANG <CODE>`
  * `LANG ENG` -> Sets controller to English.
  * `LANG TEL` -> Sets controller to Telugu.
  * `LANG HIN` -> Sets controller to Hindi.
* **Response Example**: `SF LANGUAGE UPDATED TO TELUGU.`

### 3.2 Main Control Commands
* **ON**: Turns motor ON.
* **OFF**: Turns motor OFF.
* **STATUS**: Returns average voltages, currents, water levels, soil moisture, and active state.
* **RESET**: Clears safety trip lockouts.
