#include <ESP8266Client.h>
#include <PubSubClient.h>
#include <SoftwareSerial.h>

byte server[] = { 198, 41, 30, 241 };


void callback(char* topic, byte* payload, unsigned int length) {
  // handle message arrived
}

ESP8266Client esp8266;
PubSubClient client(server, 1883, callback, esp8266);

void setup() {
  Serial.begin(9600);
  Serial.println("Starting up");
  
  // put your setup code here, to run once:
  if (client.connect("arduinoClient")) {
    //client.publish("outTopic","hello world");
    client.subscribe("inTopic");
    Serial.println("Subscribed");
  }
  Serial.println("End Setup");
}

void loop() {
  // put your main code here, to run repeatedly: 
  //client.loop();
}
