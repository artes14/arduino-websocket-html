
#include <WebSocketsClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>

#include <ArduinoJson.h>

WebSocketsClient webSocket; // websocket client class instance
StaticJsonDocument<100> doc; // Allocate a static JSON document


#ifndef APSSID
#define APSSID "ESP8266-hw"
#define APPSK  "kwakhw"
#endif

// network settings - ssid, pw, server port #
const char *ssid = APSSID;
const char *password = APPSK;

// LED
int ledPin = D2;
int ledVal=0;
int value=LOW;

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  if (type == WStype_TEXT)
  {
    // deserialize incoming Json String
    DeserializationError error = deserializeJson(doc, payload); 
    if (error) { // Print erro msg if incomig String is not JSON formated
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.c_str());
      return;
    }
    const String pin_stat = doc["PIN_Status"]; // String variable tha holds LED status
    const float val = doc["pot"]; // Float variable that holds temperature
    Serial.println(String(pin_stat)); // Print the received data for debugging
    Serial.println(String(val));
    // webSocket.sendTXT("OK"); // Send acknowledgement
    /* LED: OFF
       TMP: Temperature
       Hum: Humidity
    */
    ledVal=map(int(val), 0,1023,0,255);
    analogWrite(ledPin,ledVal);
  }
}

void setup() {
  // esp settings
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  
  WiFi.begin(ssid);
  Serial.begin(115200);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP()); // Print local IP address
  delay(1000);
  
  //address, port, and URL path
  webSocket.begin("192.168.4.1", 81, "/");
  // WebSocket event handler
  webSocket.onEvent(webSocketEvent);
  // if connection failed retry every 5s
  webSocket.setReconnectInterval(100);
}

void loop() {
  webSocket.loop();
}
