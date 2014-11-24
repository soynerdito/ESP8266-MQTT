#include <ESP8266Client.h>
#include <PubSubClient.h>
#include <SoftwareSerialLocal.h>

//MQTT Server IP Address
byte server[] = { 198, 41, 30, 241 };


SoftwareSerialLocal wifiPort(10, 11); // RX, TX
void(* resetFunc) (void) = 0;//declare reset function at address 0

void clearScr(){
  for( int i=0;i<50;i++){
    Serial.println("\n");
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  // handle message arrived
  Serial.println("Call back");
  Serial.write(payload, length);
  Serial.println();
}

ESP8266Client esp8266;
PubSubClient client(server, 1883, callback, esp8266);

boolean connectSubscribe(){
  Serial.println("calling connect");
  if (client.connect("arduinoClient")) {
    Serial.println("client connected OK");
    client.subscribe("arduino.casa");
    Serial.println("Think it is subscribed");
    return true;
  }else{
    Serial.println("Fail to connect");
    return false;
  }
}
void setup() {
  Serial.begin(9600);
  Serial.println("Starting up");
  
  //Set Software serial reference to library
  wifiPort.begin(9600);
  esp8266.setSerial( &wifiPort );
  if( esp8266.connectAP("SSID","PASSWORD") ){
    Serial.println("Connected to Wifi");
  }else{
    Serial.println("Error Connecting to Wifi");
    Serial.println("No need to continue Reset!!!");
    delay(3000);
    clearScr();
    resetFunc();
  }    
  Serial.println("calling arduino connect");
  if( connectSubscribe() ){
    
    Serial.println("Subscribed");
  }else{
    Serial.println("***************");    
    Serial.println("Forget about it reset");    
    delay(500);
    clearScr();
    resetFunc();
  }
  Serial.println("End Setup");
}

void loop() {

	//If connection fail re-connects
  if( !client.connected()){
    if( !client.connected()  )
      connectSubscribe();
  }
  client.loop();
  delay(150);
}
