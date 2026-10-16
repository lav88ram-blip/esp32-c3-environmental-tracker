/*
  Airscope | ESP32-C3 Super Mini + DHT11 | AP-hosted dashboard + WebSocket
  Wiring: DHT11 VCC -> 3V3, DATA -> GPIO 3, GND -> GND.
  The ESP32 always creates its own Wi-Fi access point; no router credentials needed.
  Required libraries: DHT sensor library by Adafruit, Adafruit Unified Sensor,
  and WebSockets by Markus Sattler (Links2004).
  Upload the dashboard files in the data/ folder to ESP32 LittleFS.
*/
#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <LittleFS.h>
#include <DHT.h>

#define DHT_PIN 3
#define DHT_TYPE DHT11
const unsigned long SAMPLE_INTERVAL_MS = 2500;
const char *AP_SSID = "AirScope-ESP32";
const char *AP_PASSWORD = "airscope123";

DHT dht(DHT_PIN, DHT_TYPE);
WebServer httpServer(80);
WebSocketsServer webSocket(81);
unsigned long lastSample = 0;
float latestTemperature = NAN;
float latestHumidity = NAN;
bool hasReading = false;

void sendReading(uint8_t clientId) {
  if (!hasReading) return;
  String json = "{\\"temperature\\":" + String(latestTemperature, 1)
    + ",\\"humidity\\":" + String(latestHumidity, 1)
    + ",\\"timestamp\\":" + String((uint64_t)millis()) + "}";
  webSocket.sendTXT(clientId, json);
}

void onWebSocketEvent(uint8_t clientId, WStype_t type, uint8_t * payload, size_t length) {
  (void)payload; (void)length;
  if (type == WStype_CONNECTED) {
    Serial.printf("WebSocket client %u connected.\\n", clientId);
    sendReading(clientId);
  } else if (type == WStype_DISCONNECTED) {
    Serial.printf("WebSocket client %u disconnected.\\n", clientId);
  }
}

void serveFile(const char * path, const char * contentType) {
  File file = LittleFS.open(path, "r");
  if (!file) {
    httpServer.send(404, "text/plain", "Dashboard file not found. Upload the data/ folder to LittleFS.");
    return;
  }
  httpServer.streamFile(file, contentType);
  file.close();
}

void setupHttp() {
  httpServer.on("/", HTTP_GET, [](){ serveFile("/index.html", "text/html; charset=utf-8"); });
  httpServer.on("/index.html", HTTP_GET, [](){ serveFile("/index.html", "text/html; charset=utf-8"); });
  httpServer.on("/style.css", HTTP_GET, [](){ serveFile("/style.css", "text/css; charset=utf-8"); });
  httpServer.on("/script.js", HTTP_GET, [](){ serveFile("/script.js", "application/javascript; charset=utf-8"); });
  httpServer.onNotFound([](){ httpServer.send(404, "text/plain", "Not found"); });
  httpServer.begin();
}

void startAccessPoint() {
  WiFi.mode(WIFI_AP);  // AP-only: never join a router or home Wi-Fi.
  if (!WiFi.softAP(AP_SSID, AP_PASSWORD)) {
    Serial.println("ERROR: Could not start Wi-Fi access point.");
    return;
  }
  Serial.println("Access point started.");
  Serial.printf("Wi-Fi network: %s\\n", AP_SSID);
  Serial.printf("Wi-Fi password: %s\\n", AP_PASSWORD);
  Serial.print("Dashboard: http://");
  Serial.println(WiFi.softAPIP());
}

void setup() {
  Serial.begin(115200);
  delay(300);
  dht.begin();

  if (!LittleFS.begin(true)) {
    Serial.println("LittleFS mount failed. Upload dashboard files to the device.");
  }

  startAccessPoint();
  setupHttp();
  webSocket.begin();
  webSocket.onEvent(onWebSocketEvent);
  Serial.println("HTTP server listening on port 80.");
  Serial.println("WebSocket server listening on port 81.");
}

void loop() {
  httpServer.handleClient();
  webSocket.loop();

  if (lastSample == 0 || millis() - lastSample >= SAMPLE_INTERVAL_MS) {
    lastSample = millis();
    float humidity = dht.readHumidity();
    float temperature = dht.readTemperature();
    if (isnan(humidity) || isnan(temperature)) {
      Serial.println("DHT11 read failed; check wiring and GPIO 3.");
    } else {
      latestHumidity = humidity;
      latestTemperature = temperature;
      hasReading = true;
      String json = "{\\"temperature\\":" + String(temperature, 1)
        + ",\\"humidity\\":" + String(humidity, 1)
        + ",\\"timestamp\\":" + String((uint64_t)millis()) + "}";
      webSocket.broadcastTXT(json);
      Serial.println(json);
    }
  }
}
