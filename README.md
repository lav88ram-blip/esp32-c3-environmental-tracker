# Airscope — ESP32-C3 WebSocket Environmental Tracker

A DHT11 dashboard that streams readings **directly over WebSockets from the ESP32-C3**. The ESP32 creates its own Wi-Fi network and hosts the website. No home router, internet connection, cloud API, MQTT, or polling service is needed.

## Architecture

- The ESP32 always runs in **Wi-Fi access-point-only mode**. It does not try to connect to a home router.
- Join the ESP32 network: **SSID `AirScope-ESP32`**, password **`airscope123`**.
- Open the dashboard at **http://192.168.4.1/**. The ESP32 serves the site from its LittleFS filesystem over HTTP port 80.
- The dashboard opens a direct WebSocket to **`ws://192.168.4.1:81/`**. The ESP32 pushes fresh DHT11 JSON readings about every 2.5 seconds.
- GitHub Pages publishes a **static dashboard preview** from `site/`. It cannot directly read the ESP32's local `ws://` socket from an HTTPS page.
- For live sensor readings, connect to the ESP32 Wi-Fi network and open `http://192.168.4.1/`. Remote live readings require a secure WebSocket relay; GitHub Pages alone does not provide one.

## Files

- `site/`: GitHub Pages static dashboard and deployment target
- `data/`: dashboard files served by the ESP32 LittleFS filesystem
- `data/`: copies served from the ESP32 LittleFS filesystem
- `firmware/esp32_dht11_tracker.ino`: AP-only HTTP + WebSocket server and DHT11 firmware
- `docs/wiring.md`: sensor wiring

## GitHub Pages deployment

A GitHub Actions workflow at `.github/workflows/pages.yml` deploys the `site/` directory whenever it changes on `main`. To enable it, open the repository **Settings → Pages**, set the build and deployment source to **GitHub Actions**, then open the Actions tab and wait for the `Deploy Airscope dashboard to GitHub Pages` workflow to succeed. The published URL should be `https://lav88ram-blip.github.io/esp32-c3-environmental-tracker/`. The hosted page is a static preview, not a live remote sensor feed.

## 1. Wire the DHT11

For a typical three-pin breakout: VCC → 3V3, DATA → GPIO 3, GND → GND. Check the labels on your module. See [docs/wiring.md](docs/wiring.md).

## 2. Configure Arduino IDE

1. Install ESP32 board support by Espressif.
2. Install **DHT sensor library by Adafruit**, **Adafruit Unified Sensor**, and **WebSockets by Markus Sattler (Links2004)** from Library Manager.
3. Open `firmware/esp32_dht11_tracker.ino`, select your ESP32-C3 board and serial port, then upload the sketch. **No `secrets.h` or Wi-Fi credentials are needed**; the device always creates its own Wi-Fi network.
4. Upload the `data/` directory to the ESP32 LittleFS filesystem using a LittleFS filesystem uploader compatible with your Arduino-ESP32 board package. The device must contain `/index.html`, `/style.css`, and `/script.js`.

## 3. Connect to the dashboard

1. Open Serial Monitor at **115200 baud** and reset the board. It should print the access-point name and dashboard IP.
2. On your phone or computer, connect to Wi-Fi **`AirScope-ESP32`** using password **`airscope123`**.
3. If your phone reports “no internet,” choose to stay connected to this network. Internet is not required for the local dashboard.
4. Open **http://192.168.4.1/** in your browser. The page connects to the WebSocket at port 81 automatically and reconnects if the connection drops.

## Security notes

- The access-point password shown here is a simple setup default. Change `AP_PASSWORD` in the firmware to a strong unique password before sharing or deploying the device.
- The dashboard uses local HTTP and plain WebSocket. Do not expose the device's ports directly to the public internet.
- The DHT11 is an approximate hobby sensor, not a safety-critical instrument.
