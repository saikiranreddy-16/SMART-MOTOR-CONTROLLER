# Installation & Wiring Guide (Version 1.0 Commercial Edition)

This guide provides step-by-step wiring schematics and initial boot instructions for installing the Smart Agricultural Motor Controller.

---

## 1. Electrical Wiring Diagrams

> [!WARNING]
> **High Voltage Hazard**: Always turn off the main circuit breaker (MCB) before connecting wires to the pump starter box. Wiring must comply with regional agricultural electrical safety codes.

### 1.1 Three-Phase AC Starter Panel (DOL & Star-Delta)

Connect the controller terminal block directly to the contactor coil and incoming line transformers:

```text
    Contactor Coil Power (415V AC Phase-to-Phase or 230V AC Phase-to-Neutral)
    
    Starter Panel                                              Controller Board
    ─────────────                                              ────────────────
    Incoming L1 (Red Phase)   ──────────────────────────────►  PIN_ADC_VOLT_R
    Incoming L2 (Yellow Phase) ──────────────────────────────►  PIN_ADC_VOLT_Y
    Incoming L3 (Blue Phase)  ──────────────────────────────►  PIN_ADC_VOLT_B
    
    Contactor Start Button    ──────[ Relay 1 Contact (NO) ]──► PIN_RELAY_START
    Contactor Run Button      ──────[ Relay 2 Contact (NO) ]──► PIN_RELAY_RUN
```

* **DOL Starters**: Wire only `PIN_RELAY_RUN` contacts in parallel across the green physical start button of the starter.
* **Star-Delta Starters**: Wire `PIN_RELAY_START` to the Star contactor coil and `PIN_RELAY_RUN` to the Delta contactor coil. The firmware handles the 4-second transition delay automatically.

### 1.2 Solar VFD Pump Panel

VFD panels do not use high-current AC contactors. They use low-voltage logic contacts:

```text
    Solar VFD Panel                                            Controller Board
    ───────────────                                            ────────────────
    FORWARD (FWD) Terminal    ──────[ Relay 2 Contact (NO) ]──► PIN_RELAY_RUN
    COMMON (COM) Terminal     ────────────────────────────────► Common Ground
    VFD Fault Relay (COM)     ────────────────────────────────► Ground
    VFD Fault Relay (NO)      ──────[ Pull-Up Input Pin ]─────► PIN_FLOAT_LOW
```

* **Start/Stop Control**: The VFD runs when the dry contact relay on `PIN_RELAY_RUN` closes. Bypasses Star-Delta timing.
* **Telemetry**: AC line sensors (ZMPT101B) are bypassed. VFD faults are monitored through the fault contact input.

---

## 2. Sensor Installations

### 2.1 Water Level Hydrostatic Depth Probe
1. Drop the IP68 hydrostatic pressure probe into the open well, lake, or farm pond.
2. Ensure the probe rests near the bottom but away from mud/silt to prevent port blockages.
3. Route the shielded cable to the controller enclosure and connect the Modbus RS485 lines:
   * **A (TX+)** $\to$ Controller Terminal RS485 A
   * **B (RX-)** $\to$ Controller Terminal RS485 B
   * **GND** $\to$ Controller Shield Ground

---

## 3. Smart Display RF Remote Pairing Procedure

To pair a new Smart Display Remote with the main base controller card:
1. Ensure the main controller is powered ON.
2. Press and hold the **PAIR** button on the handheld Smart Remote for 3 seconds.
3. The remote screen will display: `"Pairing with Base..."` and transmit pairing packets.
4. The base controller will receive the packet. If it does not have a paired remote bound in flash, it will save the remote's unique ID to NVS and reply with a confirmation.
5. The remote screen will update to display: `"PAIRED"` and start polling telemetry.
