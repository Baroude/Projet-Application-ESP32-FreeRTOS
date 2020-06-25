#include <WiFi.h>
#include <SPIFFS.h>


const char *ssid = "networkESP32";
const char *password= "azerty1234";

const char *indexHTML = "/index.html";

const int led = 2;

WiFiServer server(80);

String header;

void setup() {


  Serial.begin(9600);

  //GPIO
  pinMode(led,OUTPUT);
  digitalWrite(led,LOW);

  if(!SPIFFS.begin())
  {
    Serial.println("SPIFFS ERROR...");
    return;
  }

  //WIFI

  
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
  server.begin();
}

void loop() {
  WiFiClient client = server.available();

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
              /* digitalWrite(led, HIGH);
              delay(600);
              digitalWrite(led,LOW); */
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