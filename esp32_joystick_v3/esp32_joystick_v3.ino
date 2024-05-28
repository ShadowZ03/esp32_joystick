#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>

// SSID and password of Wifi connection:
const char* ssid = "YourSSID";
const char* password = "YourPassword";

// Define LED pins
const int ledPinN = 13;
const int ledPinE = 14;
const int ledPinS = 15;
const int ledPinW = 16;

// Initialization of webserver and websocket
WebServer server(80);                                 
WebSocketsServer webSocket = WebSocketsServer(81);    

void setup() {
  Serial.begin(115200);                              
  pinMode(ledPinN, OUTPUT);
  pinMode(ledPinE, OUTPUT);
  pinMode(ledPinS, OUTPUT);
  pinMode(ledPinW, OUTPUT);

  WiFi.softAP(ssid, password);
  Serial.println("Access Point Started");
  Serial.println(WiFi.softAPIP());

  if (!SPIFFS.begin(true)) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
  
  server.on("/", HTTP_GET, []() {                              
    File file = SPIFFS.open("/index.html", "r");
    server.streamFile(file, "text/html");
    file.close();
  });
  server.begin();                                     
  webSocket.begin();                                  
  webSocket.onEvent(webSocketEvent);                  
}

void loop() {
  server.handleClient();                              
  webSocket.loop();                                   
}

void webSocketEvent(byte num, WStype_t type, uint8_t * payload, size_t length) {
  switch (type) {
    case WStype_DISCONNECTED:
      Serial.println("Client " + String(num) + " disconnected");
      break;
    case WStype_CONNECTED:
      Serial.println("Client " + String(num) + " connected");
      break;
    case WStype_TEXT:
      StaticJsonDocument<200> doc;
      DeserializationError error = deserializeJson(doc, payload);
      if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return;
      } else {
        int g_x = doc["x"];
        int g_y = doc["y"];
        Serial.println("Received action from: " + String(num));
        Serial.println("x: " + String(g_x));
        Serial.println("y: " + String(g_y));

        // Determine LED states based on joystick input
        bool north = g_y > 100;
        bool south = g_y < -100;
        bool east = g_x > 100;
        bool west = g_x < -100;

        // Set LED states
        digitalWrite(ledPinN, north ? HIGH : LOW);
        digitalWrite(ledPinS, south ? HIGH : LOW);
        digitalWrite(ledPinE, east ? HIGH : LOW);
        digitalWrite(ledPinW, west ? HIGH : LOW);

        // For debugging
        Serial.println("LED States:");
        Serial.println("North: " + String(north));
        Serial.println("South: " + String(south));
        Serial.println("East: " + String(east));
        Serial.println("West: " + String(west));

        serializeJsonPretty(doc, Serial);
        Serial.println();
      }
      break;
  }
}
