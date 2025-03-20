#include <M5Stack.h>
#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid     = "TP-Link_146C";
const char* password = "52777882";
const char* deployid = "AKfycbyn3lh2EIiGJLmwycbL_wMe4bBCWjd01676xPmYAMv5kteDBWWmkTetyJap5VSfnLDI";
uint32_t    interval = 30; // unit:sec
uint16_t    chipid   = ESP.getEfuseMac() % 10000;

void setup() {
  M5.begin();
  M5.Lcd.setTextSize(2);
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.print("Connecting to WiFi ");
  WiFi.begin(ssid, password);
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    M5.Lcd.print(".");
  }
  M5.Lcd.println(" WiFi connected");
}

void loop() {
  M5.update();
  float val0 = random(0, 1000) / 10.0;
  float val1 = random(0, 1000) / 10.0;
  float val2 = random(0, 1000) / 10.0;
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.print("ESP");
  M5.Lcd.println(chipid);
  M5.Lcd.printf("[val0] %.1f\n[val1] %.1f\n[val2] %.1f\n", val0, val1, val2);
  addRowToGoogleSheets(val0, val1, val2);
  delay(interval * 1000);
}

void addRowToGoogleSheets(float val0, float val1, float val2) {
  HTTPClient http;
  String url = "https://script.google.com/macros/s/" + String(deployid) + "/exec?chipid=ESP" + String(chipid) + "&val0=" + String(val0) + "&val1=" + String(val1) + "&val2=" + String(val2);
  Serial.println("[HTTP] begin...");
  Serial.println(url);
  http.begin(url);
  Serial.println("[HTTP] GET...");
  int httpCode = http.GET();
  if(httpCode > 0) {
    Serial.printf("[HTTP] GET... code: %d\n", httpCode);
    if(httpCode == HTTP_CODE_OK) {
      String payload = http.getString();
      Serial.println(payload);
    }
  } else {
    Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    M5.Lcd.println("[HTTP] GET failed");
  }
  http.end();
}



//const char* ssid     = "TP-Link_146C";
//const char* password = "52777882";