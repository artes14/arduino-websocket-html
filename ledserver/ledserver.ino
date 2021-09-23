#include <WebSocketsServer.h>
#include <ArduinoJson.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>


#ifndef APSSID
#define APSSID "ESP8266-hw"
#define APPSK  "kwakhw"
#endif

// network settings - ssid, pw, server port #
const char *ssid = APSSID;
const char *password = APPSK;

// POT
int potPin = A0;
int potVal=0;
int value=LOW;

int interval = 100; // virtual delay
unsigned long previousMillis = 0; // Tracks the time since last event fired

String web="<!DOCTYPE html><html><head> <title>Arduino and ESP32 Websocket</title> <meta name='viewport' content='width=device-width, initial-scale=1.0' /> <meta charset='UTF-8'> <style> body { background-color: #E6D8D5; text-align: center; } </style></head><body> <h1>POTENTIOMETER : <span id='pot'>-</span></h1> <h1>ESP state : <span id='message'>-</span></h1><button type='button' id='BTN_1'> <h1>ON</h1> </button><button type='button' id='BTN_2'> <h1>OFF</h1> </button></body><script> var Socket; document.getElementById('BTN_1').addEventListener('click', button_1_pressed); document.getElementById('BTN_2').addEventListener('click', button_2_pressed); function init() { Socket = new WebSocket('ws://' + window.location.hostname + ':81/'); Socket.onmessage = function(event) { processCommand(event); }; } function processCommand(event) { var obj = JSON.parse(event.data); document.getElementById('message').innerHTML = obj.PIN_Status; document.getElementById('pot').innerHTML = obj.pot; console.log(obj.PIN_Status); console.log(obj.pot); } function button_1_pressed() { Socket.send('1'); } function button_2_pressed() { Socket.send('0'); } window.onload = function(event) { init(); }</script></html>";

String jsonString; // Temporary storage for the JSON String
String pin_status = ""; // Holds the status of the pin
float pot; // holds the pot value

ESP8266WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

void setup() {
  pinMode(D2, OUTPUT);
  Serial.begin(115200);
  Serial.println("");
  // wifi settings as AP
  Serial.println("Configuring access point...");
  /* You can remove the password parameter if you want the AP to be open. */
  WiFi.softAP(ssid);
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());
    // Initialize a web server on the default IP address. and send the webpage as a response.
  server.on("/", []() {
    server.send(200, "text\html", web);
  });
  server.begin(); // init the server
  webSocket.begin();  // init the Websocketserver
  webSocket.onEvent(webSocketEvent);  // init the webSocketEvent function when a websocket event occurs 
}

void loop() {
  server.handleClient();  // webserver methode that handles all Client
  webSocket.loop(); // websocket server methode that handles all Client
  unsigned long currentMillis = millis(); // call millis  and Get snapshot of time
  if ((unsigned long)(currentMillis - previousMillis) >= interval) { // How much time has passed, accounting for rollover with subtraction!
    update_pot(); // update temperature data.
    if(pin_status=="ON")update_webpage();
    previousMillis = currentMillis;   // Use the snapshot to set track time until next event
  }
}

void webSocketEvent(byte num, WStype_t type, uint8_t * payload, size_t length) {
  switch (type) {
    case WStype_DISCONNECTED: // enum that read status this is used for debugging.
      Serial.print("WS Type ");
      Serial.print(type);
      Serial.println(": DISCONNECTED");
      break;
    case WStype_CONNECTED:  // Check if a WebSocket client is connected or not
      Serial.print("WS Type ");
      Serial.print(type);
      Serial.println(": CONNECTED");
      if (digitalRead(D2) == HIGH) {  //check if pin 22 is high or low
        pin_status = "ON";
        update_webpage(); // update the webpage accordingly
      }
      else {                          
        pin_status = "OFF"; //check if pin 22 is high or low
        update_webpage();// update the webpage accordingly
      }
      break;
    case WStype_TEXT: // check responce from client
      Serial.println(); // the payload variable stores teh status internally
      Serial.println(payload[0]);
      if (payload[0] == '1') { 
        pin_status = "ON";
        update_webpage();
        digitalWrite(D2, HIGH);           
      }
      if (payload[0] == '0') {
        pin_status = "OFF";
        update_webpage();
        digitalWrite(D2, LOW);             
      }
      break;
  }
}
void update_pot(){
  pot=analogRead(potPin);
  Serial.println(pot);
}
void update_webpage()
{
  StaticJsonDocument<100> doc;
  // create an object
  JsonObject object = doc.to<JsonObject>();
  object["PIN_Status"] = pin_status ;
  object["pot"] = pot ;
  serializeJson(doc, jsonString); // serialize the object and save result to string variable.
  Serial.println( jsonString ); // print the string for debugging.
  webSocket.broadcastTXT(jsonString); // send the JSON object through the websocket
  jsonString = ""; // clear the String.
}
