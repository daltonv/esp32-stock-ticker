#include <WiFi.h>
#include <WiFiProv.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
 
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SH1107 display = Adafruit_SH1107(64, 128, &Wire);

unsigned long previousTime = 0;
const unsigned long eventInterval=50000;   //600000 is 10 minutes

const char * pop = "abcd1234"; // Proof of possession - otherwise called a PIN - string provided by the device, entered by the user in the phone app
const char * service_name = "PROV_123"; // Name of your device (the Espressif apps expects by default device name starting with "Prov_")
const char * service_key = NULL; // Password used for SofAP method (NULL = no password needed)
bool reset_provisioned = false; // When true the library will automatically delete previously provisioned data.

// Sample uuid that user can pass during provisioning using BLE
uint8_t uuid[16] = {0xb4, 0xdf, 0x5a, 0x1c, 0x3f, 0x6b, 0xf4, 0xbf,
                    0xea, 0x4a, 0x82, 0x03, 0x04, 0x90, 0x1a, 0x02 };

bool connected = false;
String payload = "";

const char* ssid = YOUR_SSID_HERE;
const char* password = YOUR_PASSWORD_HERE;

const float START_DATE_PRICE = 6.62;
const float RETIRE_DATE_PRICE = 166.85;

void resetDisplay() {
  display.clearDisplay();
  display.setTextColor(SH110X_WHITE);
  display.setCursor(0, 0);
}

HTTPClient http;


// WARNING: SysProvEvent is called from a separate FreeRTOS task (thread)!
void SysProvEvent(arduino_event_t *sys_event){
    switch (sys_event->event_id) {
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
        resetDisplay();
        display.print("\nConnected IP address : ");
        display.println(IPAddress(sys_event->event_info.got_ip.ip_info.ip.addr));
        display.display();
        connected = true;
        break;
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
        resetDisplay();
        display.println("\nDisconnected. Connecting to the AP again... ");
        display.display();
        connected = false;
        break;
    case ARDUINO_EVENT_PROV_START:
        resetDisplay();
        display.println("\nProvisioning started\nGive Credentials of your access point using smartphone app");
        display.display();
        break;
    case ARDUINO_EVENT_PROV_CRED_RECV: {
        resetDisplay();
        display.println("\nReceived Wi-Fi credentials");
        display.print("\tSSID : ");
        display.println((const char *) sys_event->event_info.prov_cred_recv.ssid);
        display.print("\tPassword : ");
        display.println((char const *) sys_event->event_info.prov_cred_recv.password);
        display.display();
        break;
    }
    case ARDUINO_EVENT_PROV_CRED_FAIL: {
        resetDisplay();
        display.println("\nProvisioning failed!\nPlease reset to factory and retry provisioning\n");
        if(sys_event->event_info.prov_fail_reason == WIFI_PROV_STA_AUTH_ERROR)
            display.println("\nWi-Fi AP password incorrect");
        else
            display.println("\nWi-Fi AP not found....Add API \" nvs_flash_erase() \" before beginProvision()");
        break;
        display.display();
    }
    case ARDUINO_EVENT_PROV_CRED_SUCCESS:
        resetDisplay();
        display.println("\nProvisioning Successful");
        display.display();
        break;
    case ARDUINO_EVENT_PROV_END:
        resetDisplay();
        display.println("\nProvisioning Ends");
        display.display();
        break;
    default:
        break;
    }
}

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
 
void readPrice(int x, int y) {
  
  int httpCode;

  httpCode = http.GET();
 
  if (httpCode > 0) {
    DynamicJsonDocument doc(1024);
    String payload = http.getString();
    Serial.println(payload);
    deserializeJson(doc, payload);
 
    float previousClosePrice = doc["pc"];
    float currentPrice = doc["c"];
    float differenceInPrice = ((currentPrice - previousClosePrice) / previousClosePrice) * 100.0;
    float differenceSinceStart = ((currentPrice - START_DATE_PRICE) / START_DATE_PRICE) * 100.0;
    float differenceSinceRetire = ((currentPrice - RETIRE_DATE_PRICE) / RETIRE_DATE_PRICE) * 100.0;
 
    resetDisplay();
    display.setTextSize(2);
    display.setCursor(x, y);
    display.println("PG");
    display.print(currentPrice, 2);
    display.println(" USD");
 
    display.setTextSize(1);

    display.print("Last Close: ");
    display.print(differenceInPrice, 2);
    display.println("%");

    display.print("Start Date: ");
    display.print(differenceSinceStart, 2);
    display.println("%");

    display.print("Retire Date: ");
    display.print(differenceSinceRetire, 2);
    display.println("%");
 
    display.display();
  } else {
    resetDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("Error in HTTP request");
    display.print("Error code: ");
    display.println(httpCode);
    display.display();
  }
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

  String httpRequestAddress = "https://finnhub.io/api/v1/quote?symbol=PG&token=c1tjb52ad3ia4h4uee9g";
  http.begin(httpRequestAddress);
  // WiFi.onEvent(SysProvEvent);

  // Serial.println("Begin Provisioning using BLE");
  // WiFiProv.beginProvision(WIFI_PROV_SCHEME_BLE, WIFI_PROV_SCHEME_HANDLER_FREE_BTDM, WIFI_PROV_SECURITY_1, pop, service_name, service_key, uuid, reset_provisioned);
  // log_d("ble qr");
  // WiFiProv.printQR(service_name, pop, "ble");

  // display.println("To provision Wifi connect serial monitor");
  // display.display();
}
 
void loop() {
  unsigned long currentTime = millis(); 

  if (((currentTime - previousTime) >= eventInterval) || previousTime == 0)
  {
    readPrice(0,0);
    previousTime = currentTime;
  }
}