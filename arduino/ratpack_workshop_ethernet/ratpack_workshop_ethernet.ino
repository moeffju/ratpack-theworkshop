#include <SPI.h>
#include <Ethernet.h>
#include "Config.h"

/*
* rat pack workshop result
*
* basically this is occupy ping pong with a push button replacing the infrared motion sensor,
* see https://github.com/makersandco/Occupy-Ping-Pong for details.
*
* sven kraeuter for makers & co.
*
* http://makersand.co
*
* wifly library for arduino 1.0: https://github.com/timr/WiFly-Shield
*
*/

byte mac[] = {  0xDE, 0xAD, 0xBE, 0xEF, 0xBA, 0xBE };

EthernetClient client;

void setup() {
  //------------------------ led setup -------------------------
  pinMode(ledPin, OUTPUT);      
  //----------------------- button setup -----------------------
  pinMode(buttonPin, INPUT);    
  // ---------------------- serial setup -----------------------
  Serial.begin(9600);
  
  // Initialize Ethernet shield
  delay(1000);
  Serial.println("Connecting to Ethernet...");
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Ethernet initialization failed.");
    while (1) { /* Hang on failure */ }
  }

  Serial.print(sprintf("Connecting to %s:80", HOST));
  if (client.connect(HOST, 80)) {
    Serial.println(" connected!");
  }
  else {
    Serial.println(" failed!");
    while(1) { /* fail */ }
  }
}

void loop() {
  //------------------- button part ---------------------------------------
  if (digitalRead(buttonPin) == LOW) {
    Serial.println("Button NOT pressed.");
    //--------- send status to server
    sendRequest(generateHttpPut(HOST, RESOURCE, '0'));
  }
  else if (digitalRead(buttonPin) == HIGH) {
    Serial.println("Button IS pressed!");
    //--------- send status to server 
    sendRequest(generateHttpPut(HOST, RESOURCE, '1'));
  }
  
  //-------------------- signalling led part -------------------------------------
  char postString[255];
  sprintf(postString, "GET %s HTTP/1.1\r\nUser-Agent: %s\r\nHost: %s\r\n", RESOURCE, USERAGENT, HOST);
  sendRequest(postString);
  
  if (client.available()) {
    // receiving status from server
    if(client.find("\"activated\": ")){
      char state[1];
      client.readBytes(state, 1);
      if(atoi(state) == 1){
        Serial.println("BUTTON PRESSED!");
        digitalWrite(ledPin, HIGH);
      }
      else if(atoi(state) == 0) {
        Serial.println("button not pressed.");
        digitalWrite(ledPin, LOW);
      }
    } 
    if (!client.connected()) {
      Serial.println();
      Serial.println("disconnecting.");
      client.stop();
    }
    
  }
  delay(pause); 
}

char* generateHttpPut(char* host, char* resource, char occupied) {
  char postString[255];
  sprintf(postString, "PUT %s HTTP/1.1\r\nUser-Agent: %s\r\nHost: %s\r\nContent-Length: 16\r\nContent-Type: application/json\r\n\r\n{\"activated\": %c}"
  , resource, USERAGENT, host, occupied);

  return postString;
}

void sendRequest(char* http_request){
  if (!client.connected()) {
    Serial.println();
    Serial.println("Disconnected, stopping client.");
    client.stop();
    Serial.print("Attempting to reconnect...");
    if (client.connect(HOST, 80)) {
      Serial.println(" success!");
    }
    else {
      Serial.println(" failed!");
    }
  }

  Serial.println(http_request);
  client.print(http_request);
  client.println();
  
  while (!client.available()) {
    //Serial.print('.');
  }
  Serial.println();
}
