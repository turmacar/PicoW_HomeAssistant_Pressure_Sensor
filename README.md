# Raspberry Pi Pico W Pressure Sensor for Home Assistant

A pressure sensor project for Raspberry Pi Pico W using Home Assistant and MQTT. Reads a 0–200 psi analog pressure sensor via ADC, applies voltage divider compensation, and publishes readings to Home Assistant every 5 seconds.

This project is built on the [arduino-pico](https://github.com/earlephilhower/arduino-pico) core and [arduino-home-assistant](https://github.com/dawidchyrzynski/arduino-home-assistant) libraries for local integration.

## Features

- **Analog pressure sensor** (0–200 psi, 0.5–4.5 V output)
- **Voltage divider circuit** (1/3 ratio: 2 kΩ + 1 kΩ) to protect ADC from overvoltage
- **16-sample averaging** for stable readings
- **MQTT auto-discovery** - sensor appears in Home Assistant automatically
- **Board LED control** - toggle from Home Assistant dashboard for testing
- **~30 KB flash, 28% RAM** - plenty of headroom for additional features

## Dependencies

- [Raspberry Pi Pico W](https://www.raspberrypi.com/products/raspberry-pi-pico/?variant=raspberry-pi-pico-w) - $6 WiFi-enabled microcontroller
- [PlatformIO](https://platformio.org/platformio-ide) - VSCode IDE for embedded development
- [`arduino-pico`](https://github.com/earlephilhower/arduino-pico) - Arduino core for RP2040 boards
- [`arduino-home-assistant`](https://github.com/dawidchyrzynski/arduino-home-assistant) - MQTT integration library
- `PubSubClient` - MQTT client (via PlatformIO)

## Code Structure

- **`src/HAIntegration.h`** - Sensor configuration, ADC constants, device class
- **`src/HAIntegration.cpp`** - `readPressurePsi()` (voltage divider, conversion), sensor publishing
- **`src/Network.cpp`** - WiFi connection
- **`src/main.cpp`** - Setup/loop entry points
- **`src/Credentials.h`** - WiFi + MQTT credentials template

## Setup & Installation

### Prerequisites
- Raspberry Pi Pico W
- PlatformIO IDE (VSCode extension)
- Home Assistant + MQTT broker on local network
- Analog pressure sensor (0–200 psi, 0.5–4.5 V output)
- Wiring: 2 kΩ resistor + 1 kΩ resistor (voltage divider)
- Optional: 100 nF capacitor for ADC filtering

### Hardware Wiring

Connect the pressure sensor to Pico W as follows:

| Sensor | → | Pico W Pin | Function |
|---|---|---|---|
| Red (IN+) | → | VBUS (pin 40) | 5 V power |
| Black (GND) | → | GND (pin 38) | Ground |
| Green (OUT) | → | GP26/ADC0 via divider | ADC input (pin 31) |

**Voltage divider:** Sensor OUT → 2kΩ → GP26/ADC0 → 1kΩ → AGND (pin 33)

The required pins are on the Pico's right-hand edge:

```text
+--------------------------------+
| PICO W                         |
| USB connector at top           |
|                                |
| Right edge:                    |
| VBUS       pin 40 -- RED       |
| VSYS       pin 39 -- unused    |
| GND        pin 38 -- BLACK     |
| AGND       pin 33 -- 1 kΩ end  |
| GP26/ADC0  pin 31 -- junction  |
+--------------------------------+
```

Complete circuit:

```text
                         +---------------- VBUS, pin 40
                         |
                    RED  |
                  +------+------+
                  |  PRESSURE   |
                  |   SENSOR    |
                  +------+------+
                    BLACK|GREEN
                         |  |
GND, pin 38 -------------+  |
                            |
                          [2 kΩ]
                            |
                            +------------- GP26/ADC0, pin 31
                            |
                          [1 kΩ]
                            |
AGND, pin 33 --------------+
```

With USB disconnected:

1. Connect sensor **red** to the row containing **VBUS pin 40**.
2. Connect sensor **black** to the row containing **GND pin 38**.
3. Put sensor **green** into an unused row.
4. Connect the **2 kΩ resistor** from the green-wire row to the **GP26 pin 31**
   row.
5. Connect the **1 kΩ resistor** from the GP26 row to the **AGND pin 33** row.
6. Optionally connect a **100 nF capacitor** between the GP26 and AGND rows.

```text
GP26 row: [Pico pin 31] [2 kΩ end] [1 kΩ end] [optional capacitor]
AGND row: [Pico pin 33] [1 kΩ end]              [optional capacitor]
```

Before connecting USB, verify that VBUS/sensor red is not connected directly
to GP26. With no pressure applied, expect approximately **0.5 V from sensor
green to GND** and **0.167 V from GP26 to AGND**.


### Breadboard layout
A working layout on a standard breadboard (columns a–e | trench | f–j, with
+/− power rails). The Pico straddles the trench with its USB end at row 30,
so its right-edge pins land in column **g**:

| Hole | Connection |
|---|---|
| g30 | Pico VBUS (pin 40) |
| g28 | Pico GND (pin 38) |
| g23 | Pico AGND (pin 33) |
| g21 | Pico GP26/ADC0 (pin 31) |
| i30 → + rail | Jumper: VBUS to + rail |
| i28 → − rail | Jumper: GND to − rail |
| + rail / − rail | Sensor red / sensor black |
| i2 | Sensor green |
| h2 → e3 | Jumper: sensor green across the trench to row 3 |
| d3 – d5 | 2 kΩ resistor |
| b5 – b7 | 1 kΩ resistor |
| e5 → i21 | Jumper: divider midpoint to GP26 |
| a7 → i23 | Jumper: 1 kΩ bottom to AGND |

```text
         a    b    c    d    e  |  f    g    h    i    j
row 2                           |            jmp  GRN        <- sensor green
row 3                 2k   jmp  |                            <- from h2
row 5         1k      2k   jmp  |                            -> i21 (GP26)
row 7    jmp  1k                |                            -> i23 (AGND)
  ...
row 21                          |       GP26      jmp        <- from e5
row 23                          |       AGND      jmp        <- from a7
row 28                          |       GND       jmp        -> - rail
row 30                          |       VBUS      jmp        -> + rail
```

Rows 3, 5 and 7 are the only nodes in the divider: row 3 = sensor out,
row 5 = ADC input (sensor out / 3), row 7 = analog ground.

On long breadboards the power rails are often split at the midpoint; keep the
VBUS/GND jumpers and the sensor leads on the same rail segment.

Measured on this build (no pressure, USB power): sensor green ≈ **0.491 V**,
GP26 ≈ **0.164 V**. The firmware zero is calibrated to this (see
[Calibration](#calibration)).

### Firmware Build & Upload

1. **Set up credentials:**
   ```bash
   cd PicoW_HomeAssistant_Pressure_Sensor
   cp src/Credentials.h src/Credentials.local.h
   # Edit Credentials.local.h with WiFi + MQTT details
   ```

2. **Build firmware:**
   ```bash
   source .venv/bin/activate  # or create with: python3 -m venv .venv && pip install platformio
   pio run
   ```
   Output: `.pio/build/pico/firmware.uf2` (768 KB)

3. **Upload to Pico W:**
   - Hold **BOOTSEL** button while connecting via USB
   - Pico W appears as mass storage (`RPI-RP2`)
   - Copy `firmware.uf2` to the drive
   - Board reboots automatically and starts reading sensor

4. **Verify in Home Assistant:**
   - Settings → Devices & Services → MQTT
   - Click `X devices discovered`
   - Find `Pico W Pressure Sensor` device
   - Under **Entities** → `Pressure` sensor (in psi)
   - Optional: `Board LED` switch for testing

### Serial Monitor (Optional)

Watch pressure readings and MQTT status:
```bash
pio run -t monitor
```

Expected output:
```
Connecting to MQTT
Connected to MQTT broker
Pressure: 0.45 psi
Pressure: 0.47 psi
...
```

## Sensor & ADC Math

**Sensor spec:** Linear 0–200 psi → 0.5–4.5 V output  
**Pico W ADC:** 12-bit, 3.3 V reference (exposed inputs GP26–GP28)

**Challenge:** 4.5 V sensor output exceeds 3.3 V ADC max  
**Solution:** Voltage divider (1/3 ratio) reduces peak voltage:

```
4.5 V × (1 kΩ / 3 kΩ) = 1.5 V  ✓ Safe for ADC
```

**Conversion pipeline:**
```c
analogReadResolution(12)                           // arduino-pico defaults to 10-bit
adcVoltage = analogRead(pin) * (3.3 / 4095.0)     // 0–3.3V
sensorVoltage = adcVoltage * 3.0                   // Undo divider
psi = (sensorVoltage - ZERO_V) * (200.0 / 4.0)    // Linear scale to 0–200, ZERO_V defaults to 0.5
psi = constrain(psi, 0.0, 200.0)                   // Clamp
```

### Calibration

The firmware assumes the spec zero of **0.5 V = 0 psi** (`PRESSURE_ZERO_V`).
Individual sensors and USB supplies vary slightly, so you can override it in
`platformio.ini`:

```ini
build_flags =
	-DPRESSURE_ZERO_V=0.49f
```

To find your value, open the serial monitor with nothing connected to the
sensor port and read `sensorV` from the `ADC raw=...` debug line. Remove the
flag to go back to 0.5 V.


# Appendix: Setting up a local Home Assistant installation with Mosquitto

## Home Assistant install

Home Assistant can be [installed](https://www.home-assistant.io/installation/) several ways on all kinds of hardware. Raspberry Pi is a popular choice, but you can also use your existing computer for initial explorations. *Home Assistant Container* is probably the quickest and most portable way to get started, whichever hardware you're using, though this does mean you'll have to manage some add-ons yourself, including MQTT.

## MQTT and Mosquitto

MQTT provides realtime messaging between devices in a network. The [Eclipse Mosquitto](https://github.com/eclipse/mosquitto) project offers a server that can act as an _MQTT broker_, a central hub for MQTT devices to send and receive data.

If you're using **Home Assistant Operating System**, check these instructions to [install the Mosquitto MQTT broker add-on](https://github.com/home-assistant/addons/blob/master/mosquitto/DOCS.md).

If you're using **Home Assistant Container**, [this tutorial](https://hometechhacker.com/mqtt-home-assistant-using-docker-eclipse-mosquitto/) provides a great overview on how to set up and secure Mosquitto in a seperate container.

Once your MQTT broker is enabled, [add the MQTT integration to your Home Assistant instance](https://www.home-assistant.io/integrations/mqtt/).

# Pull requests

PRs are welcome.

---

Questions? Comments? Gripes? Open an [issue](https://github.com/daniloc/PicoW_HomeAssistant_Starter/issues/new/choose) if you think there will be general interest or impact on the project.
