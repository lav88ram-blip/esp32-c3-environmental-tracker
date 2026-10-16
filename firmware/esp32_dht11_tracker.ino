/*
  Airscope | ESP32-C3 Super Mini + DHT11 | Local Wi-Fi dashboard + WebSocket
  Wiring: DHT11 VCC -> 3V3, DATA -> GPIO 3, GND -> GND.
  The ESP32 joins your existing 2.4 GHz Wi-Fi network. Edit WIFI_SSID and WIFI_PASSWORD below before uploading.
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
const char *WIFI_SSID = "YOUR_WIFI_NAME";
const char *WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

DHT dht(DHT_PIN, DHT_TYPE);
WebServer httpServer(80);
WebSocketsServer webSocket(81);
unsigned long lastSample = 0;
float latestTemperature = NAN;
float latestHumidity = NAN;
bool hasReading = false;

String readingJson() {
  return "{\"temperature\":" + String(latestTemperature, 1)
    + ",\"humidity\":" + String(latestHumidity, 1)
    + ",\"timestamp\":" + String((uint64_t)millis()) + "}";
}

void sendReading(uint8_t clientId) {
  if (hasReading) {
    String payload = readingJson();
    webSocket.sendTXT(clientId, payload);
  }
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

void connectToWiFi() {
  WiFi.mode(WIFI_STA);  // Join your existing local Wi-Fi network.
  WiFi.setAutoReconnect(true);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.printf("Connecting to Wi-Fi: %s\\n", WIFI_SSID);

  unsigned long startAttempt = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttempt < 20000) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Wi-Fi connected.");
    Serial.print("Dashboard: http://");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("ERROR: Wi-Fi connection failed. Check WIFI_SSID/WIFI_PASSWORD and reset the board.");
  }
}

void setup() {
  Serial.begin(115200);
  delay(300);
  dht.begin();

  if (!LittleFS.begin(true)) {
    Serial.println("LittleFS mount failed. Upload dashboard files to the device.");
  }

  connectToWiFi();
  setupHttp();
  webSocket.begin();
  webSocket.onEvent(onWebSocketEvent);
  Serial.println("HTTP server listening on port 80.");
  Serial.println("WebSocket server listening on port 81.");
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    static unsigned long lastReconnectAttempt = 0;
    if (millis() - lastReconnectAttempt >= 10000) {
      lastReconnectAttempt = millis();
      Serial.println("Wi-Fi disconnected; retrying...");
      WiFi.disconnect();
      WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    }
  }
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
      String json = readingJson();
      webSocket.broadcastTXT(json);
      Serial.println(json);
    }
  }
}
