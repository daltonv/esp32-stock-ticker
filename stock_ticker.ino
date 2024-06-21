#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
 
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SH1107 display = Adafruit_SH1107(64, 128, &Wire);
 
const char* ssid = "HalloHalloHallo";
const char* password = "TestTestTestTestTest";
String payload = "";
 
void connectWiFi() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(0, 0);
  Serial.println("Connecting to WiFi...");
  display.display();
 
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
  }
 
  display.clearDisplay();
  display.setCursor(0, 0);
  Serial.println("Connected to WiFi");
  display.display();
  delay(2000);
}
 
void resetDisplay() {
  display.clearDisplay();
  display.setTextColor(SH110X_WHITE);
  display.setCursor(0, 0);
}
 
void readPrice(int x, int y, const String& stockName) {
  String httpRequestAddress = "https://finnhub.io/api/v1/quote?symbol=" + stockName + "&token=c1tjb52ad3ia4h4uee9g";
  HTTPClient http;
  int httpCode;
 
  http.begin(httpRequestAddress);
  httpCode = http.GET();
 
  if (httpCode > 0) {
    DynamicJsonDocument doc(1024);
    String payload = http.getString();
    Serial.println(payload);
    deserializeJson(doc, payload);
 
    float previousClosePrice = doc["pc"];
    float currentPrice = doc["c"];
    float differenceInPrice = ((currentPrice - previousClosePrice) / previousClosePrice) * 100.0;
 
    resetDisplay();
    display.setTextSize(2);
    display.setCursor(x, y);
    display.print(stockName);
    Serial.println(stockName);
 
    if (differenceInPrice < 0.0) {
      display.setTextColor(SH110X_WHITE);
    } else {
      display.setTextColor(SH110X_WHITE);
    }
 
    display.setTextSize(2);
    display.setCursor(x, y + 25);
    display.print(currentPrice, 2);
    Serial.println(" USD");
 
    display.setTextSize(2);
    display.setCursor(x, y + 50);
    display.print(differenceInPrice, 2);
    Serial.println("%");
 
    display.display();
  } else {
    resetDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    Serial.println("Error in HTTP request");
    display.display();
  }
 
  http.end();
}
 
void setup() {
  Serial.begin(115200);
  Wire.setPins(23, 22);
  display.begin(0x3C, true); // Address 0x3C default
  display.display();
  delay(1000);
  display.clearDisplay();
  display.display();
  display.setRotation(1);
  display.setTextColor(SH110X_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  Serial.println("Stock Prices Tracker");
  display.display();
 
  connectWiFi();
}
 
void loop() {
  display.clearDisplay();
  readPrice(0,0, "PG");
  delay(3000);
  display.clearDisplay();
}