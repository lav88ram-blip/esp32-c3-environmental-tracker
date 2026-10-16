# Wiring: ESP32-C3 Super Mini + DHT11

## Three-pin DHT11 module

| DHT11 module pin | ESP32-C3 Super Mini |
|---|---|
| VCC / + | 3V3 |
| DATA / OUT / S | GPIO 3 |
| GND / - | GND |

```text
ESP32-C3 Super Mini             DHT11 module
┌─────────────────┐             ┌────────────┐
│ 3V3 ──────────────────────────│ VCC / +   │
│ GPIO 3 ───────────────────────│ DATA / OUT│
│ GND ──────────────────────────│ GND / -   │
└─────────────────┘             └────────────┘
```

**Check the labels printed on your exact module.** A bare four-pin DHT11 has a different layout from many three-pin breakout modules. For a common bare sensor viewed from the front grille, the pins are VCC, DATA, NC, GND; verify the datasheet for your part. Bare sensors may need a 4.7–10 kΩ pull-up resistor between DATA and 3V3; many breakout boards already include one.

Power the sensor from **3.3 V** so the data signal is safe for the ESP32-C3. The firmware uses GPIO 3.

## Direct dashboard + WebSocket

The ESP32 always starts its own Wi-Fi access point; it does not connect to a home router. Connect your phone or computer to **AirScope-ESP32** (password **airscope123**) and open **http://192.168.4.1/**. The ESP32 serves the dashboard over HTTP port 80 and streams JSON readings over WebSocket at **ws://192.168.4.1:81/**. The dashboard files must be uploaded to the ESP32's LittleFS filesystem. GitHub Pages stores the source but is not the live dashboard endpoint.

The default access-point password is intended for setup only; change `AP_PASSWORD` in the firmware before sharing or deploying the device. The DHT11 is a low-cost, low-precision sensor. Avoid direct sunlight, nearby heat sources, and condensation.
