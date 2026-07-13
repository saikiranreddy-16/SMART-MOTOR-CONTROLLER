# Smart Agricultural Motor Controller Backend (Version 1.0 Commercial Edition)

This is the complete, production-ready NestJS 10.x enterprise backend for the Smart Agricultural Motor Controller.

---

## 1. System Requirements
* Node.js v20.x or higher
* PostgreSQL v15 or higher
* Redis v7.x or higher
* Eclipse Mosquitto or compatible MQTT Broker

---

## 2. Local Setup & Installation

1. Install dependencies:
   ```bash
   npm install
   ```

2. Create and edit `.env` from the template:
   ```bash
   cp .env.example .env
   ```

3. Seed the database with sample mock data (Farmers, Devices, whitelisted members, CT/PT values):
   ```bash
   npm run seed
   ```

4. Run the development server:
   ```bash
   npm run start:dev
   ```

---

## 3. Deployment with Docker Compose

To start the complete stack (NestJS API, Postgres Database, Redis Cache, Mosquitto MQTT) with a single command:

```bash
docker-compose up -d --build
```

---

## 4. API & MQTT Documentation

* **Swagger Documentation**: Accessible locally at [http://localhost:3000/api](http://localhost:3000/api).
* **Health Check**: Exposed at `/health`.
* **MQTT Subscription Channels**:
  * Telemetry updates: `device/{deviceId}/telemetry`
  * Commands dispatch: `device/{deviceId}/command`
  * Status updates: `device/{deviceId}/status`
  * Firmware check: `device/{deviceId}/ota`
