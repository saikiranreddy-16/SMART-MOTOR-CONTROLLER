# Hardware & PCB Schematic Design Guide (Version 1.0 Commercial Edition)

This guide specifies the industrial electrical design, protection networks, sensor interface circuits, and PCB layout specifications for the Version 1.0 Commercial Edition Smart Agricultural Motor Controller.

---

## 1. Variable Frequency Drive (VFD) Solar Pump Interface

Solar pumps use Variable Frequency Drives (VFDs) that regulate AC output dynamically using PV panel DC voltage. The controller interacts with the VFD via isolated dry-contacts and feedback lines.

```text
  ESP32 Pin 19 ───[ Optocoupler PC817 ]───[ ULN2003 ]───[ 12V Relay ]───► [ VFD RUN Terminal ]
                                                                        ► [ VFD COM Terminal ]
                                                                        
  VFD FAULT OUT (Relay COM/NO) ───[ 10K Pull-Up ]───[ PC817 Opto ]──────► ESP32 Pin 25 (GPIO)
```

1. **VFD Start/Stop Triggering**: The run coil relay (`PIN_RELAY_RUN`) closes the contact across the VFD's digital control inputs (typically labeled `COM` and `DI1`/`FWD`). This activates VFD startup parameters bypassing standard Star-Delta high-current sequences.
2. **VFD Fault Monitoring**: VFD controllers output an internal trip status (COM/NO dry contact). We route this output through an isolated optocoupler to ESP32 digital input pin `PIN_FLOAT_LOW` (or dedicated VFD fault pin) to immediately halt operations on over-current or over-temperature warnings inside the VFD.

---

## 2. CC1101 Smart RF Remote SPI Interface

The Smart Display Remote uses a CC1101 Sub-1GHz RF transceiver. It communicates with the ESP32 using the VSPI hardware bus:

```text
    ESP32 Pin (GPIO)             CC1101 Pin
    ────────────────             ──────────
    CS (GPIO 5)   ──────────────► CSN (Chip Select)
    SCK (GPIO 18)  ──────────────► SCK (SPI Clock)
    MISO (GPIO 19) ◄────────────── SO (MISO / GDO1)
    MOSI (GPIO 23) ──────────────► SI (MOSI)
    GDO0 (GPIO 4)  ◄────────────── GDO0 (Asserts on Sync Word)
```

* **RF Shielding**: The CC1101 module must be placed inside a shielded section of the main controller enclosure, with its RF output connected to an external high-gain omnidirectional dipole antenna using an IP65 SMA bulkhead pigtail connector.
* **GDO0 Pin**: Serves as a hardware interrupt. When a valid sync word is matched by the demodulator, GDO0 pulls high, waking up the ESP32's RF task to process the packet immediately.

---

## 3. Modbus RS485 Sensor Conditioning

To read depth sensors in lakes and farm ponds (hydrostatic pressure level transmitters) reliably over long cables (up to 500 meters), we use RS485 Modbus.

```text
  ESP32 TX2 (Pin 14) ────► [ TXD ]   SP3485   [ A ] ────► [ Twist Pair Cable A ]
  ESP32 RX2 (Pin 13) ◄──── [ RXD ] Transceiver [ B ] ────► [ Twist Pair Cable B ]
  ESP32 DE/RE (Pin 15) ──► [ DE/RE ]
```

* **Transceiver (SP3485)**: A 3.3V low-power RS485 transceiver maps TTL UART levels to differential RS485 lines.
* **TVS Protection**: Placed inline on A and B lines (e.g. **SM712** TVS diode) to clamp differential transient spikes from lightning strikes in open fields.
* **Termination**: A $120\,\Omega$ termination resistor is placed across A and B lines at the end of the bus to prevent signal reflections.

---

## 4. Enclosure & Mechanical Specifications

The commercial enclosure is engineered for harsh outdoor environments:
1. **Compact, Handheld Size**: Dimensioned at $160\text{mm} \times 110\text{mm} \times 60\text{mm}$ to easily mount next to standard starter panels.
2. **IP65 Ingress Rating**: A silicone gasket seal runs around the perimeter of the lid. PG9 and PG11 cable glands clamp the incoming phase and sensor wires.
3. **Mounting Versatility**: Built-in rear mounting brackets support wall mounting via anchor plugs, and a molded backplate slot accepts standard 35mm DIN rails.
4. **Antenna Routing**: External waterproof SMA bulkhead connectors route both the GSM helical antenna and the RF 433MHz antenna outside the enclosure body for max signal propagation.
