# Smart Remote Specification (Version 1.0 Commercial Edition)

This document details the hardware design, battery management, display layout, and pairing logic for the handheld Smart Display RF Remote.

---

## 1. Handheld Unit Overview

The Smart Display Remote allows farmers to walk up to 500 meters away from the motor starter panel and query status or trigger controls.

```text
       ┌──────────────────────────────────┐
       │   BATT:95%             PAIRED    │  ◄── Header Row
       ├──────────────────────────────────┤
       │   PUMP STATUS:                   │
       │   RUNNING                        │  ◄── Big Status Text
       │                                  │
       │   WATER: 82%     POWER: YES      │  ◄── Sensor Readings
       └──────────────────────────────────┘
```

---

## 2. Power & Battery Management

To maximize operating cycles between charges, the remote incorporates dedicated battery conditioning circuitry:

### 2.1 Charging Subsystem
* **Charger IC**: **TP4056** $1\text{A}$ Linear Li-Ion Battery Charger with thermal regulation.
* **Input Interface**: USB Type-C port ($5\text{V}$ input).
* **LED Indicators**: 
  * Red LED: Charging in progress.
  * Blue/Green LED: Fully charged.

### 2.2 Low-Power Standby Mode
* **Sleep States**: The ATmega328P is configured to sleep using the `SLEEP_MODE_PWR_DOWN` registers.
* **Wake-Up Triggers**: Pressing any button (Start, Stop, Pair) triggers an external interrupt (`INT0` or `INT1`), waking the processor instantly to transmit command packets.
* **Idle Timeout**: If no buttons are pressed for 15 seconds, the display is turned off (`SSD1306_DISPLAYOFF` I2C command) and the MCU returns to deep sleep.
* **Typical Current Draw**:
  * Active Transmitting: $25\text{mA}$
  * Standby Sleep: $< 15\mu\text{A}$

---

## 3. Remote PCB & Component Placement

* **Transceiver Module**: CC1101 Sub-1GHz GFSK wireless module. Placed opposite the lithium battery cell to minimize RF attenuation.
* **Antenna**: $433\text{MHz}$ internal helical copper spring antenna ($2.15\text{dBi}$ gain) integrated inside the remote casing.
* **Enclosure**: Ergonomic ABS plastic enclosure, IP54 rated (dust and splash resistant), with an integrated keychain loop.
