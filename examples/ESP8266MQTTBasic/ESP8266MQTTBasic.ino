#include <ESP8266Client.h>
#include <PubSubClient.h>
#include <SoftwareSerialLocal.h>

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
}

ESP8266Client esp8266;
PubSubClient client(server, 1883, callback, esp8266);

boolean connectSubscribe(){
  Serial.println("calling arduino connect");
  if (client.connect("arduinoClient")) {    
    //client.publish("arduino.esp8266.out","Connected");
    client.subscribe("arduino.esp8266");
    return true;
  }else{
    return false;
  }
}
void setup() {
  Serial.begin(57600);
  Serial.println("Starting up");
  
  //Set Software serial reference to library
  wifiPort.begin(9600);
  esp8266.setSerial( &wifiPort );
  if( esp8266.connectAP("SSID","WIFI_PASSWORD") ){
    Serial.println("Connected to Wifi");
  }else{
    Serial.println("Error Connecting to Wifi");
    Serial.println("No need to continue Reset!!!");
    delay(6000);
    clearScr();
    resetFunc();
  }    
  delay(500);
  
  // put your setup code here, to run once:
  Serial.println("calling arduino connect");
  if( connectSubscribe() ){
    Serial.println("Subscribed");
  }else{
    Serial.println("***************");    
    Serial.println("Forget it reset");
    delay(30000);
    clearScr();
    resetFunc();
  }
  Serial.println("End Setup");
}

void loop() {
  // put your main code here, to run repeatedly: 
  Serial.println("loop");
  if( !client.loop() ){
    connectSubscribe();
  }
  delay(100);
}
