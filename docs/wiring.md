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

Power the sensor from **3.3 V** so the data signal is safe for the ESP32-C3. The example firmware uses GPIO 3.

## WebSocket data

The ESP32 hosts the dashboard over HTTP port 80 and streams JSON sensor readings over WebSocket port 81 on the local network. The browser connects to `ws://<ESP32-IP>:81/`. GitHub Pages can store the source files but cannot host the device's WebSocket server; use the dashboard served by the ESP32 for this direct connection.

The DHT11 is a low-cost, low-precision sensor. Avoid direct sunlight, nearby heat sources, and condensation.