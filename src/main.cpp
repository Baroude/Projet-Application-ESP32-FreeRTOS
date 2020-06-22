#include <WiFi.h>
void wifiConnection(void *pvParameters);

const char* ssid = "networkESP32";
const char* pass = "azerty1234";

void setup() {
  Serial.begin(9600);
  xTaskCreate(wifiConnection, "wifi connection", 4096, NULL, 1,NULL);
}

void loop() {
  delay(2000);
}

void wifiConnection(void *pvParameters) {

  Serial.print("Connection to ");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED ) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");
  Serial.print("IP address is ");
  Serial.println(WiFi.localIP());
  for ( ;; ) {
    delay(1000);
  }
}