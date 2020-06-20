#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>


const char *ssid = "networkESP32";
const char *password= "azerty";

const int led = 2;

AsyncWebServer server(80);
void setup() {


  Serial.begin(115200);

  //GPIO
  pinMode(led,OUTPUT);
  digitalWrite(led,LOW);

  //WIFI

  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  //Server
   server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    digitalWrite(led,HIGH);
    delay(500);
    digitalWrite(led,LOW);
    Serial.println("Requête d'index.html, led clignote");
    request->send(SPIFFS, "/index.html", "text/html");
  });

  server.begin();
}

void loop() {
  // put your main code here, to run repeatedly:
}