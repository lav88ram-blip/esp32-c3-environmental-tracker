# Airscope — ESP32-C3 Environmental Tracker

A responsive GitHub Pages dashboard and Arduino firmware for an **ESP32-C3 Super Mini + DHT11** temperature/humidity monitor. The dashboard reads history from ThingSpeak over HTTPS.

## Project files

- `index.html`, `style.css`, `script.js` — static dashboard
- `firmware/esp32_dht11_tracker.ino` — ESP32-C3 firmware
- `firmware/secrets.example.h` — safe configuration template
- `docs/wiring.md` — wiring guide

## 1. Wire the sensor

For a typical three-pin DHT11 breakout module: VCC to 3V3, DATA to GPIO 4, GND to GND. Check the module's labels and read [the wiring guide](docs/wiring.md) before powering it.

## 2. Create a ThingSpeak channel

1. Sign in at [ThingSpeak](https://thingspeak.com/).
2. Create a channel and enable **Field 1** (Temperature C) and **Field 2** (Humidity %).
3. Save the channel. Copy its numeric **Channel ID** and its **Write API Key**.
4. Make the channel **public** if you want the GitHub Pages dashboard to read it without a read key. Do not publish your Write API Key.

## 3. Configure and flash the ESP32-C3

1. Install Arduino IDE and add ESP32 board support from Espressif.
2. Install **DHT sensor library by Adafruit** (and the Adafruit Unified Sensor dependency) through Library Manager.
3. Open `firmware/esp32_dht11_tracker.ino`.
4. Copy `firmware/secrets.example.h` to `firmware/secrets.h` and edit the copy with your Wi-Fi SSID/password and ThingSpeak **Write API Key**. The real `secrets.h` is ignored by Git.
5. Select an ESP32-C3-compatible board/port and upload. Open Serial Monitor at **115200 baud**.
6. The sketch uploads readings approximately every 20 seconds. Keep the interval at least 15 seconds for a free ThingSpeak channel.

**Do not put your Wi-Fi password or ThingSpeak write key in `script.js`, HTML, or any public repository file.** If a password was shared publicly or committed, change it.

## 4. Connect the dashboard

1. In ThingSpeak, confirm that Field 1 and Field 2 are receiving values.
2. Open the dashboard and enter your numeric channel ID in the **ThingSpeak Channel ID** box, then click **Connect**.
3. The ID is saved in your browser's local storage. The dashboard refreshes about every 30 seconds.
4. If the page says it cannot load readings, make sure the channel is public and both fields have data.

## 5. Publish with GitHub Pages

1. In this repository, open **Settings → Pages**.
2. Under Build and deployment, choose **Deploy from a branch**.
3. Choose branch `main` and folder `/ (root)`, then save.
4. Wait for the Pages deployment to finish. The site should be available at:

   https://lav88ram-blip.github.io/esp32-c3-environmental-tracker/

GitHub Pages hosts the website only. It does not run code on the ESP32; the ESP32 sends readings to ThingSpeak, and the static site reads the channel.

## Notes

- This dashboard's chart overlays humidity on the same visual range as temperature to compare trends; use the numeric cards for exact values.
- DHT11 readings are approximate and intended for hobby monitoring, not safety-critical measurement.
- The firmware uses HTTPS with certificate verification disabled for a compact prototype. For a production device, validate the server certificate.
