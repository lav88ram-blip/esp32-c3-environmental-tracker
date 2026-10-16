/*
  Airscope | ESP32-C3 Super Mini + DHT11 | WebSocket-only
  Wiring: DHT11 VCC -> 3V3, DATA -> GPIO 3, GND -> GND.
  Wi-Fi credentials in secrets.h are optional; without them, the device starts its setup AP.
  Required libraries: DHT sensor library by Adafruit, Adafruit Unified Sensor,
  and WebSockets by Markus Sattler (Links2004).
  Upload the dashboard files in the data/ folder to ESP32 LittleFS.
*/
#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <LittleFS.h>
#include <DHT.h>
// A local secrets.h is optional. Never commit real Wi-Fi credentials.
#if __has_include("secrets.h")
  #include "secrets.h"
#else
  #define WIFI_SSID ""
  #define WIFI_PASSWORD ""
#endif

#define DHT_PIN 3
#define DHT_TYPE DHT11
const unsigned long SAMPLE_INTERVAL_MS = 2500;
DHT dht(DHT_PIN, DHT_TYPE);
WebServer httpServer(80);
WebSocketsServer webSocket(81);
unsigned long lastSample = 0;
float latestTemperature = NAN;
float latestHumidity = NAN;
bool hasReading = false;

void sendReading(uint8_t clientId) {
  if (!hasReading) return;
  String json = "{\"temperature\":" + String(latestTemperature, 1)
    + ",\"humidity\":" + String(latestHumidity, 1)
    + ",\"timestamp\":" + String((uint64_t)millis()) + "}";
  webSocket.sendTXT(clientId, json);
}
void onWebSocketEvent(uint8_t clientId, WStype_t type, uint8_t * payload, size_t length) {
  (void)payload; (void)length;
  if (type == WStype_CONNECTED) {
    Serial.printf("WebSocket client %u connected.\n", clientId);
    sendReading(clientId);
  } else if (type == WStype_DISCONNECTED) {
    Serial.printf("WebSocket client %u disconnected.\n", clientId);
  }
}
void serveFile(const char * path, const char * contentType) {
  File file = LittleFS.open(path, "r");
  if (!file) { httpServer.send(404, "text/plain", "Dashboard file not found. Upload the data/ folder to LittleFS."); return; }
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
  WiFi.mode(WIFI_AP);
  WiFi.softAP("AirScope-ESP32", "airscope123");
  Serial.println("Setup access point started.");
  Serial.println("Connect to Wi-Fi AirScope-ESP32 (password: airscope123).");
  Serial.print("Dashboard: http://");
  Serial.println(WiFi.softAPIP());
}

void connectWiFi() {
  // A missing or untouched example secrets.h should not prevent compilation.
  String ssid = String(WIFI_SSID);
  String password = String(WIFI_PASSWORD);
  bool credentialsConfigured = ssid.length() > 0 && password.length() > 0
    && ssid != "YOUR_WIFI_NAME" && password != "YOUR_WIFI_PASSWORD";

  if (!credentialsConfigured) {
    Serial.println("No Wi-Fi credentials configured; starting setup access point.");
    startAccessPoint();
    return;
  }

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  unsigned long started = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - started < 20000) {
    delay(400); Serial.print(".");
  }
  Serial.println();
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("Dashboard: http://"); Serial.println(WiFi.localIP());
  } else {
    Serial.println("Station Wi-Fi failed; starting setup access point.");
    startAccessPoint();
  }
}
void setup() {
  Serial.begin(115200); delay(300); dht.begin();
  if (!LittleFS.begin(true)) Serial.println("LittleFS mount failed. Upload dashboard files to the device.");
  connectWiFi();
  setupHttp();
  webSocket.begin();
  webSocket.onEvent(onWebSocketEvent);
  Serial.println("WebSocket server listening on port 81.");
}
void loop() {
  httpServer.handleClient();
  webSocket.loop();
  if (WiFi.status() != WL_CONNECTED && WiFi.getMode() != WIFI_AP) {
    static unsigned long lastReconnect = 0;
    if (millis() - lastReconnect > 10000) { lastReconnect = millis(); WiFi.reconnect(); }
  }
  if (lastSample == 0 || millis() - lastSample >= SAMPLE_INTERVAL_MS) {
    lastSample = millis();
    float humidity = dht.readHumidity();
    float temperature = dht.readTemperature();
    if (isnan(humidity) || isnan(temperature)) {
      Serial.println("DHT11 read failed; check wiring and GPIO 3.");
    } else {
      latestHumidity = humidity; latestTemperature = temperature; hasReading = true;
      String json = "{\"temperature\":" + String(temperature, 1)
        + ",\"humidity\":" + String(humidity, 1)
        + ",\"timestamp\":" + String((uint64_t)millis()) + "}";
      webSocket.broadcastTXT(json);
      Serial.println(json);
    }
  }
}