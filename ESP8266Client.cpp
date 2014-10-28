
extern "C" {
#include "string.h"
}

#include "Arduino.h"

#include "ESP8266Client.h"

#define AT_DISCONNECT "AT+CLOSE\n"
#define AT_RESET "AT+RST\n"
#define AT_WIFI_MODE "AT+CWMODE"
#define AT_JOIN_AP "AT+CWJAP"
#define AT_LIST_AP "AT+CWLAP"
#define AT_STATUS "AT+CIPSTATUS"
#define AT_TCPSTART "AT+CIPSTART"
#define AT_CIPSEND "AT+CIPSEND"
#define AT_GET_IP "AT+CIFSR"

#define MAX_SOCK_NUM 1

uint16_t ESP8266Client::_srcport = 1024;
//AT+CIPSTART="TCP","192.168.10.117",10000
//AT+CIPSTART="TCP","192.168.10.117",10000

ESP8266Client::ESP8266Client() : _sock(MAX_SOCK_NUM) {
}

ESP8266Client::ESP8266Client(uint8_t sock) : _sock(sock) {
}

int ESP8266Client::connect(const char* host, uint16_t port) {
	// Look up the host first
	// No DNS implementation as of now
	return 1;
}

int ESP8266Client::connect(IPAddress ip, uint16_t port) {
	//Close anny connection
	Serial.println("AT+CIPCLOSE");
	delay(20);
	char buffer[50];
	uint8_t* rawAdress = rawIPAddress(ip);
	sprintf( buffer, "AT+CIPSTART=\"TCP\",\"%d.%d.%d.%d\",%d", rawAdress[0], rawAdress[1], rawAdress[2], rawAdress[3], port);
	Serial.println(buffer);
	Serial.setTimeout(5000);
	if( Serial.find("OK\r\n") ){
		return true;
	} else {
		return false;
	}
}

size_t ESP8266Client::write(uint8_t b) {
	return Serial.write((char)b);
}

size_t ESP8266Client::write(const uint8_t *buf, size_t size) {
	Serial.write(buf,(size/sizeof(uint8_t)));
	return size;
}

int ESP8266Client::available() {
	//Serial.println("::available");
	return 1;
}

/*
 * Received data starts with +IPD
 */
int ESP8266Client::read() {
	Serial.println("::read");
return 0;
/*uint8_t b;
 if ( recv(_sock, &b, 1) > 0 )
 {
 // recv worked
 return b;
 }
 else
 {
 // No data available
 return -1;
 }*/
}

int ESP8266Client::read(uint8_t *buf, size_t size) {
	Serial.println("::read(uint8_t");
	//return recv(_sock, buf, size);
	return 0;
}

int ESP8266Client::peek() {
	Serial.println("::peek");
	/*  uint8_t b;
	 // Unlike recv, peek doesn't check to see if there's any data available, so we must
	 if (!available())
	 return -1;
	 ::peek(_sock, &b);
	 return b;*/
	return 0;
}

void ESP8266Client::flush() {
	Serial.println("::flush");
	while (available())
	read();
}

void ESP8266Client::stop() {
	Serial.println("::stop");
	Serial.write( AT_DISCONNECT );
}

uint8_t ESP8266Client::connected() {
	return sendWaitRespond("AT+CIPSTATUS", "STATUS:3", 200 );
}

uint8_t ESP8266Client::status() {
	return sendWaitRespond("AT+CIPSTATUS", "STATUS:3", 200 );
}

// the next function allows us to use the client returned by
// EthernetServer::available() as the condition in an if-statement.

ESP8266Client::operator bool() {
	return 1;
}

bool ESP8266Client::operator==(const ESP8266Client& rhs) {
	return _sock == rhs._sock && _sock != MAX_SOCK_NUM && rhs._sock != MAX_SOCK_NUM;
}

bool ESP8266Client::connectAP(char *ssid, char *password) {
	char buffer[50];
	Serial.println("AT+CWMODE=1");
	sprintf( buffer, "AT+CWJAP=\"%s\",\"%s\"", ssid, password);

	return sendWaitRespond(buffer, "OK", 5000 );
	/*Serial.println( buffer );
	delay(2000);
	if(Serial.find("OK")) {
		return true;
	} else {
		return false;
	}*/
}

bool ESP8266Client::sendWaitRespond( char *message, char *response, int msTimeout) {
	Serial.println(message);
	Serial.setTimeout(msTimeout);
	if( Serial.find(response) ){
		return true;
	} else {
		return false;
	}
}
