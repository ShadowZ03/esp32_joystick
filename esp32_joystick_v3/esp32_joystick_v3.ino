#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>                         // needed for instant communication between client and server through Websockets
#include <ArduinoJson.h>
#include <SPIFFS.h>


const char* ssid = "YourSSID";
const char* password = "YourPassword";

int interval = 1000;                                  // send data to the client every 1000ms -> 1s
unsigned long previousMillis = 0;                     // we use the "millis()" command for time reference and this will output an unsigned long

// Initialization of webserver and websocket
WebServer server(80);                                 // the server uses port 80 (standard port for websites
WebSocketsServer webSocket = WebSocketsServer(81);    // the websocket uses port 81 (standard port for websockets

void setup() {
    Serial.begin(115200);

    WiFi.softAP(ssid, password);
    Serial.println("Access Point Started");
    Serial.println(WiFi.softAPIP());

    if (!SPIFFS.begin(true)) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
    }
  
    server.on("/", HTTP_GET, []() {
        File file = SPIFFS.open("/index", "r");
        if (!file) {
            Serial.println("Failed to open file");
            server.send(404, "text/plain", "File not found");
            return;
        }
        server.streamFile(file, "text/html");
        file.close();
    });

    server.on("/joystick", HTTP_POST, []() {
        StaticJsonDocument<200> doc;
        deserializeJson(doc, server.arg("plain"));

        int x = doc["x"];
        int y = doc["y"];

        // Map joystick data to PWM values for motors
        int motor1Speed = map(x, 0, 1023, 0, 255);
        int motor2Speed = map(y, 0, 1023, 0, 255);

        // Use motor1Speed and motor2Speed to control motors with PWM
        // Code to control motors with PWM
        Serial.printf("Motor 1 Speed: %d, Motor 2 Speed: %d\n", motor1Speed, motor2Speed);

        server.send(200, "text/plain", "OK");
    });

    server.begin();
}

void loop() {
    server.handleClient();
}
