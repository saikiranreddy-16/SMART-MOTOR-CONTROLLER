# Device Enclosure Specification (Version 1.0 Commercial Edition)

This guide specifies the mechanical dimensions, panel layout, connector locations, and mounting details for the IP65-rated commercial enclosure.

---

## 1. Outer Dimensions & Mounting

* **Casing Type**: Industrial wall-mount ABS enclosure with a silicone sealing gasket.
* **Dimensions**: $165\text{mm} \times 115\text{mm} \times 60\text{mm}$ (Length $\times$ Width $\times$ Depth).
* **Mounting Options**:
  * **DIN Rail Mount**: Integrated backplate accepts standard $35\text{mm}$ DIN rails.
  * **Screw Mount**: 4 pre-molded corner screw channels for direct wall anchor mounting.

---

## 2. Panel Layout & User Interfaces

```text
  ┌────────────────────────────────────────────────────────┐
  │                                    [GSM ANT]  [RF ANT] │  ◄── Antenna Ports (SMA)
  │  ┌───────────────────────────┐                         │
  │  │   Borewell Pump           │                         │  ◄── Main OLED Screen
  │  │   State: RUNNING          │                         │
  │  └───────────────────────────┘                         │
  │                                                        │
  │  (LED 1)  (LED 2)  (LED 3)            [ MAN | AUTO ]   │  ◄── LEDs & Manual Switch
  │  Power     Run     Fault               Bypass Toggle   │
  │                                                        │
  │  [======= Screw Terminal Blocks Block J1-J4 =======]   │  ◄── Bottom Glands entry
  └────────────────────────────────────────────────────────┘
```

1. **Front Panel OLED Screen**: $1.3$ inch high-contrast white OLED screen protected by a clear acrylic window.
2. **LED Indicators**:
  * **POWER** (Green, $5\text{mm}$): Illuminated when 3.3V DC is active.
  * **RUN** (Red, $5\text{mm}$): Illuminated when the motor delta contactor is energized.
  * **FAULT** (Amber, $5\text{mm}$): Flashes when a safety trip occurs (corresponds to E001-E010 codes).
3. **Manual Bypass Switch**: Heavy-duty industrial toggle switch. In "MANUAL" mode, it bypasses the ESP32 logic entirely and returns control to physical push buttons on the starter panel.

---

## 3. Cable Entry & Connectors

* **Cable Glands (Bottom Panel)**:
  * **PG13.5 (1 Qty)**: For high-voltage 3-Phase AC lines (Red, Yellow, Blue, Neutral).
  * **PG11 (1 Qty)**: For output contactor triggers (Start, Run, Aux relays).
  * **PG9 (2 Qty)**: For external sensor cables (Modbus RS485 and water float switches).
* **SMA Bulkhead Connector (Top Panel)**:
  * **Port 1**: Waterproof female SMA connector for $4\text{G}$ cellular whip antenna.
  * **Port 2**: Waterproof female SMA connector for $433\text{MHz}$ RF remote antenna.
