# Manufacturing & Quality Guide (Version 1.0 Commercial Edition)

This guide documents the assembly, factory testing, calibration, and packaging procedures required for production line assembly.

---

## 1. PCB Assembly (PCBA) & Soldering
1. **SMD Reflow**: Reflow soldering should be performed on a 6-zone reflow oven using lead-free SAC305 solder paste.
2. **Selective Soldering**: Heavy parts (terminals J1-J4, Omron relays, potential transformers, screw blocks) must be selectively wave soldered at $260^\circ\text{C}$ to guarantee high mechanical strength.
3. **Conformal Coating**: Apply a thin acrylic conformal coating layer over the digital region (ESP32 and SIM7600) to protect the controller from humidity, dust, and corrosive vapors in rural agricultural pump sheds.

---

## 2. Factory Test Jig & Diagnostic Loop

Every board must be validated on the factory test fixture before flashing:

```text
    [ Mains Power 230V AC ] ────► [ AC Variac Controller ] ──► [ Current Transformer Load ]
                                                                       │
    [ Spring Test Pins (Pogo Pins) ] ◄─── Contact Top Layer ◄──────────┘
```

1. **Jig Interface**: Uses spring-loaded contact pins (Pogo Pins) touching the 8 PCB test points (TP1 to TP8).
2. **AC Voltage Injection**: Inject $180\text{V}$, $230\text{V}$, and $260\text{V}$ AC sequentially. The test jig reads TP8 to verify the ZMPT101B potential transformer calibration.
3. **Current Sensor Calibration**: Load the circuit with a resistive heater drawing $5\text{A}$ and $15\text{A}$ currents. Read TP7 to calibrate the CT scaling factor.

---

## 3. Factory QA Checklist (Before Packing)

Each unit must pass the validation checklist:
- [ ] **Check 1**: +3.3V supply (TP1) rests between $3.25\text{V}$ and $3.35\text{V}$.
- [ ] **Check 2**: Power-On Self-Test (POST) runs on start, returning no fault codes.
- [ ] **Check 3**: Relays engage sequentially on test command.
- [ ] **Check 4**: Cellular SIM7600 registers with the carrier within 60 seconds (RSSI $> 15$).
- [ ] **Check 5**: CC1101 pairs successfully with the test remote.
- [ ] **Check 6**: High-voltage insulation barrier tested at $2.0\text{kV}$ AC dielectric strength.

---

## 4. Packaging Specifications

* **Anti-Static Wrapping**: The board must be sealed inside a pink anti-static bubble bag.
* **Controller Carton**: Packaged inside a corrugated box ($180\text{mm} \times 130\text{mm} \times 80\text{mm}$).
* **Inbox Contents**:
  1. Main Controller Unit (IP65 Enclosure).
  2. GSM Helical Antenna.
  3. RF External Dipole Antenna.
  4. Quick Start & Wiring Manual (Multilingual: English, Telugu, Hindi).
