#include <WiFi.h>
#include <SPIFFS.h>
#include<Arduino.h>

void wifiConnection(void *pvParameters);
void clientHandle(void *param);
void fileChecker(void * param);

const char* ssid = "networkESP32";
const char* pass = "azerty1234";

const char *indexHTML = "/index.html";

const int led = 2;

WiFiServer server(80);

String header;

TaskHandle_t fileHandle = NULL;

void setup() {
  Serial.begin(115200);
  server.begin();
  xTaskCreatePinnedToCore(wifiConnection, "wifi connection", 4096, NULL, 1,NULL, 0);
  xTaskCreatePinnedToCore(clientHandle, "Handles clients", 4096,NULL, 1, NULL, 1 );
  xTaskCreatePinnedToCore(fileChecker, "Checks file system", 4096, NULL, 2, &fileHandle, 0)
}

void loop() {
  delay(2000);
}
void fileChecker(void *param){
  for ( ;; ){

  if(!SPIFFS.begin())
  {
    Serial.println("SPIFFS ERROR...");
    return;
  }

  vTaskDelete(fileHandle);
  }
}
void wifiConnection(void *pvParameters) {

  Serial.print("Connection to ");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED ) {
    vTaskDelay(500 / portTICK_PERIOD_MS);
    Serial.print(".");
  }
  Serial.println("WiFi connected");
  Serial.print("IP address is ");
  Serial.println(WiFi.localIP());
  for ( ;; ) {
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void clientHandle(void *param){

  WiFiClient client = server.available();

  for ( ;; ){
    if(client){

      Serial.println("New client connected");
      String currentLine= "";
      while (client.connected()){

        if(client.available()){

          char c = client.read();
          Serial.write(c);
          header += c;
          if (c == '\n'){
            if (currentLine.length() == 0){
              client.println("HTTP/1.1 200 OK");
              client.println("Content-type:text/html");
              client.println("Connection: close");
              client.println();

              if (header.indexOf("GET /") >= 0){
                Serial.println("HTTP GET request on /, LED blinks");
                digitalWrite(led, HIGH);
                vTaskDelay(600 / portTICK_PERIOD_MS);
                digitalWrite(led,LOW);
                File pageWeb = SPIFFS.open(indexHTML, "r");
                while (pageWeb.available()){
                    client.println(pageWeb.readString());
                } 
                pageWeb.close(); 
              
             
              }
              /*
              client.println("<!DOCTYPE html>");
              client.println("<html lang=\"fr \">");
              client.println("<head> <title> Serveur Web ESP32 </title>");
              client.println("<meta name= \"Web Server ESP32 for benchmark\" charset=\"UTF-8\">");
              client.println("<style>body{background-color:lightblue;}");
              client.println("h1{color: black;text-align: center;}</style> </head>");
              client.println("<body><h1>ESP32</h1></body> </html>");
              client.println();
              */
              break;

            }
            else{
              currentLine = "";
            }

          }
          else if (c != '\r'){
            currentLine += c;
          }
        }
      }
      header="";
      client.stop();
      Serial.println("Client disconnected.");
      Serial.println("");
      
      
    }
  }

}