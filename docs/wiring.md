# Wiring: ESP32-C3 Super Mini + DHT11

## Three-pin DHT11 module

| DHT11 module pin | ESP32-C3 Super Mini |
|---|---|
| VCC / + | 3V3 |
| DATA / OUT / S | GPIO 4 |
| GND / - | GND |

```text
ESP32-C3 Super Mini             DHT11 module
┌─────────────────┐             ┌────────────┐
│ 3V3 ──────────────────────────│ VCC / +   │
│ GPIO 4 ───────────────────────│ DATA / OUT│
│ GND ──────────────────────────│ GND / -   │
└─────────────────┘             └────────────┘
```

**Important:** Check the labels printed on your particular module. A bare four-pin DHT11 sensor has a different pin order from many three-pin breakout modules; do not assume the module layout applies to the bare sensor. With a bare sensor, looking at the front grille, the common pin order is VCC, DATA, NC, GND, but confirm its datasheet. Some bare sensors need a pull-up resistor (typically 4.7–10 kΩ) between DATA and 3V3; many breakout modules already include one.

Power the sensor from **3.3 V** to keep its data signal safe for the ESP32-C3. GPIO 4 is used in the sample firmware and can be changed in the sketch if needed.

## Data fields

- ThingSpeak Field 1: temperature in degrees Celsius
- ThingSpeak Field 2: relative humidity in percent

The DHT11 is a low-cost, relatively low-precision sensor. Avoid placing it in direct sunlight, near a heat source, or where condensation can form.