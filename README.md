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

Connect the pressure sensor to Pico W as shown in [project_context.local.md](project_context.local.md):

| Sensor | → | Pico W Pin | Function |
|---|---|---|---|
| Red (IN+) | → | VBUS (pin 3) | 5 V power |
| Black (GND) | → | GND (pin 4) | Ground |
| Green (OUT) | → | GP0 via divider | ADC input (pin 5) |

**Voltage divider:** Sensor OUT → 2kΩ → GP0 → 1kΩ → GND

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
**Pico W ADC:** 12-bit, 3.3 V reference (pins GP0–GP5)

**Challenge:** 4.5 V sensor output exceeds 3.3 V ADC max  
**Solution:** Voltage divider (1/3 ratio) reduces peak voltage:

```
4.5 V × (1 kΩ / 3 kΩ) = 1.5 V  ✓ Safe for ADC
```

**Conversion pipeline:**
```c
adcVoltage = analogRead(pin) * (3.3 / 4095.0)     // 0–3.3V
sensorVoltage = adcVoltage * 3.0                   // Undo divider
psi = (sensorVoltage - 0.5) * (200.0 / 4.0)       // Linear scale to 0–200
psi = constrain(psi, 0.0, 200.0)                   // Clamp
```

## Extending the Project

See [examples in `arduino-home-assistant`](https://github.com/dawidchyrzynski/arduino-home-assistant/tree/main/examples) for:
- Additional analog sensors
- Digital inputs (buttons, switches)
- PWM outputs (fans, lights)
- HVAC controls

# Troubleshooting

- To come

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

Questions? Comments? Gripes? Open an [issue](https://github.com/daniloc/PicoW_HomeAssistant_Starter/issues/new/choose) if you think there will be general interest or impact on the project. Otherwise hit me up: [mail@danilocampos.com](mailto:mail@danilocampos.com).
