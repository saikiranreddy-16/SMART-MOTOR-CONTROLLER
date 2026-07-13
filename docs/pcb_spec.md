# PCB Design & Layout Specifications (Version 1.0 Commercial Edition)

This document details the layout constraints, creepage distances, electromagnetic compatibility, and test points for the controller board.

---

## 1. High Voltage Isolation & Clearance

To protect low-voltage digital components (ESP32, sensors) from $415\text{V}$ AC phase voltage transients, the PCB incorporates strict physical isolation rules:

```text
    High-Voltage AC Area (415V AC)
    ──────────────────────────────────────────
    [Relays]  [Terminals]  [PTs]
    ══════════════════════════════════════════  ◄── Routing Isolation Slots (8mm physical air gap)
    [Optos]   [LDO AP2112] [ESP32]
    ──────────────────────────────────────────
    Low-Voltage DC Area (3.3V/5V DC)
```

1. **Isolation Slots**: A physical slot (cut-out channel) of $8\text{mm}$ is milled into the FR4 board directly under the optocouplers, potential transformers, and switching relays to prevent creepage currents over the surface of the board.
2. **Track Clearance**: 
  * High-Voltage AC traces (Red, Yellow, Blue phases): Minimum spacing of $4.0\text{mm}$.
  * Low-Voltage DC signal traces: Spacing of $0.2\text{mm}$ (8 mil).
3. **Trace Width**: High-voltage relay driver tracks must be at least $2.5\text{mm}$ wide with $2\text{oz}$ copper plating to carry high currents (up to $16\text{A}$) without heating.

---

## 2. Electromagnetic Compatibility (EMC) & EMI Filtering

Agricultural fields are prone to lightning and power grid surges. The PCB incorporates three protective layers:
* **Varistor Clamp Array**: Six Metal Oxide Varistors (**V20E320P**) clamp differential surge spikes between phases.
* **RC Snubber Networks**: Placed in parallel across relay contacts to absorb switching arcs when contactors disengage.
* **Ground Planes**: Dedicated ground plane on the bottom layer to act as an RF shield for the ESP32.

---

## 3. Physical Test Points (TP1 - TP8) Layout

Dedicated gold-plated circular pads ($1.5\text{mm}$ diameter) are exposed on the PCB top layer to assist manufacturing diagnostics:

* **TP1 (3.3V)**: Verifies LDO regulator output.
* **TP2 (5V)**: Verifies switching buck converter output (relay supply).
* **TP3 (12V)**: Verifies main incoming supply line.
* **TP4 (RELAY)**: Monitors ULN2003 output node.
* **TP5 (GSM_TX)** / **TP6 (GSM_RX)**: UART communication diagnostic taps.
* **TP7 (I_BIAS)**: Monitors current transformer analog bias voltage (+1.65V offset).
* **TP8 (V_BIAS)**: Monitors potential transformer analog bias voltage (+1.65V offset).
