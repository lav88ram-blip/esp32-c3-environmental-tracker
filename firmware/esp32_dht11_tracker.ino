/*
  Airscope | ESP32-C3 Super Mini + DHT11
  Wiring: DHT11 VCC -> 3V3, DATA -> GPIO 4, GND -> GND.
  Copy secrets.example.h to secrets.h and fill in your own credentials.
  Install libraries: "DHT sensor library" by Adafruit and its Adafruit Unified Sensor dependency.
  ThingSpeak channel: Field 1 = Temperature C, Field 2 = Relative Humidity %.
*/
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <DHT.h>
#include "secrets.h"

#define DHT_PIN 4
#define DHT_TYPE DHT11
const unsigned long UPLOAD_INTERVAL_MS = 20000; // ThingSpeak free channels require >= 15 seconds between updates.
DHT dht(DHT_PIN, DHT_TYPE);
unsigned long lastUpload = 0;

void connectWiFi() {
  Serial.print("Connecting to Wi-Fi");
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  unsigned long started = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - started < 25000) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("Connected. IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("Wi-Fi connection timed out; will retry.");
  }
}

bool uploadReading(float temperatureC, float humidity) {
  WiFiClientSecure client;
  // Prototype convenience: TLS encrypts traffic but does not validate the server certificate.
  // For a production installation, configure certificate validation instead of setInsecure().
  client.setInsecure();
  HTTPClient https;
  String url = String("https://api.thingspeak.com/update?api_key=") + THINGSPEAK_WRITE_API_KEY
    + "&field1=" + String(temperatureC, 1)
    + "&field2=" + String(humidity, 1);
  if (!https.begin(client, url)) {
    Serial.println("Could not start HTTPS request.");
    return false;
  }
  https.setConnectTimeout(8000);
  https.setTimeout(8000);
  int code = https.GET();
  String response = https.getString();
  https.end();
  if (code == HTTP_CODE_OK && response.toInt() > 0) {
    Serial.println("Upload accepted. Entry ID: " + response);
    return true;
  }
  Serial.printf("Upload failed. HTTP %d, response: %s\n", code, response.c_str());
  return false;
}

void setup() {
  Serial.begin(115200);
  delay(500);
  dht.begin();
  connectWiFi();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
    if (WiFi.status() != WL_CONNECTED) {
      delay(3000);
      return;
    }
  }
  if (lastUpload == 0 || millis() - lastUpload >= UPLOAD_INTERVAL_MS) {
    lastUpload = millis();
    float humidity = dht.readHumidity();
    float temperatureC = dht.readTemperature();
    if (isnan(humidity) || isnan(temperatureC)) {
      Serial.println("DHT11 read failed; check wiring and sensor pin.");
    } else {
      Serial.printf("Temperature: %.1f C | Humidity: %.1f %%\n", temperatureC, humidity);
      uploadReading(temperatureC, humidity);
    }
  }
  delay(100);
}