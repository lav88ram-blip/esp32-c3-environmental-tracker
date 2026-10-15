# Airscope — ESP32-C3 WebSocket Environmental Tracker

A DHT11 dashboard that streams readings **directly over WebSockets from the ESP32-C3**. No ThingSpeak, MQTT, cloud API, or polling service is used.

## Architecture (important)

- The ESP32 runs an HTTP server on port 80 to serve the dashboard files and a WebSocket server on port 81 to stream JSON readings.
- The browser connects to `ws://<ESP32-IP>:81/`; new readings are pushed from the device.
- GitHub is the source-code repository. **GitHub Pages cannot run your ESP32 WebSocket server.** For the direct WebSocket version, open the dashboard at the ESP32's local IP address. The GitHub Pages URL cannot connect to the device's plain `ws://` endpoint from an HTTPS page because browsers block insecure mixed content.
- This setup works on the same local Wi-Fi network. Remote access requires a separately hosted **secure WebSocket (WSS) relay** or carefully configured VPN/reverse proxy; don't expose the device's ports directly to the internet.

## Files

- `index.html`, `style.css`, `script.js`: dashboard source
- `data/`: copies served from the ESP32 LittleFS filesystem
- `firmware/esp32_dht11_tracker.ino`: HTTP + WebSocket server and DHT11 firmware
- `firmware/secrets.example.h`: safe Wi-Fi configuration template
- `docs/wiring.md`: sensor wiring

## 1. Wire the DHT11

For a typical three-pin breakout: VCC → 3V3, DATA → GPIO 4, GND → GND. Check the labels on your module. See [docs/wiring.md](docs/wiring.md).

## 2. Configure Arduino IDE

1. Install ESP32 board support by Espressif.
2. Install **DHT sensor library by Adafruit**, **Adafruit Unified Sensor**, and **WebSockets by Markus Sattler (Links2004)** from Library Manager.
3. Copy `firmware/secrets.example.h` to `firmware/secrets.h`; set your Wi-Fi name and password. This real secrets file is ignored by Git.
4. Open `firmware/esp32_dht11_tracker.ino`, select your ESP32-C3 board and serial port, then upload the sketch.
5. Upload the `data/` directory to the ESP32 LittleFS filesystem using a LittleFS filesystem uploader compatible with your Arduino-ESP32 board package. The device must contain `/index.html`, `/style.css`, and `/script.js`.

## 3. Connect to the dashboard

- Open Serial Monitor at **115200 baud**.
- If station Wi-Fi connects, note the printed dashboard address, such as `http://192.168.1.123/`.
- Open that address in a browser on the same Wi-Fi network. The dashboard connects to the WebSocket at port 81 automatically.
- If the configured Wi-Fi connection fails, the ESP32 creates a setup access point named `AirScope-ESP32` with password `airscope123`. Join it and open `http://192.168.4.1/`. Change the fallback password in the firmware before using this outside a trusted environment.
- Readings are sent immediately to connected clients whenever the sensor is sampled. The page reconnects automatically if the WebSocket drops.

## Security notes

- Do not commit Wi-Fi credentials.
- The fallback access-point password is a simple setup default; change it to a strong unique password before use.
- The local dashboard uses HTTP and plain WebSocket on the LAN. Do not port-forward these ports to the public internet.
- The DHT11 is an approximate hobby sensor, not a safety-critical instrument.