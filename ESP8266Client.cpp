
extern "C" {
#include "string.h"
}

#include "Arduino.h"

#include "ESP8266Client.h"

#define AT "AT"
#define AT_DISCONNECT "AT+CIPCLOSE"
#define AT_RESET "AT+RST"
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
char buffer[50];
SoftwareSerial *serialPort=0;

#define DEBUG_SERIAL
/*
 * Using a modified SoftwareSerial library to read directly what is on the Serial Buffer
 * Official SoftwareSerial has receive buffer private, modified to use it public
 */
ESP8266Client::ESP8266Client() : _sock(MAX_SOCK_NUM) {
}

ESP8266Client::ESP8266Client(uint8_t sock) : _sock(sock) {
}

int ESP8266Client::connect(const char* host, uint16_t port) {
	// Look up the host first
	// No DNS implementation as of now
	return 1;
}
void ESP8266Client::setSerial( SoftwareSerial *port ){
	serialPort = port;
}
int ESP8266Client::connect(IPAddress ip, uint16_t port) {
	//Close anny connection
	serialPort->println("AT+CIPCLOSE");
	delay(20);

	uint8_t* rawAdress = rawIPAddress(ip);
	sprintf( buffer, "AT+CIPSTART=\"TCP\",\"%d.%d.%d.%d\",%d", rawAdress[0], rawAdress[1], rawAdress[2], rawAdress[3], port);
	serialPort->println(buffer);
	serialPort->setTimeout(5000);
	if( serialPort->find("OK\r\n") ){
		return true;
	} else {
		return false;
	}
}

size_t ESP8266Client::write(uint8_t b) {
	serialPort->print("AT+CIPSEND=1");
	if (serialPort->find(">"))
	{
		//OK
		serialPort->write(b);
		serialPort->println("\n");
		return sizeof(uint8_t);
	}else
	{
		serialPort->println("AT+CIPCLOSE");
		return 0;
	}
}

size_t ESP8266Client::write(const uint8_t *buf, size_t size) {
	//serialPort->write(buf,(size/sizeof(uint8_t)));
	sprintf( buffer, "AT+CIPSEND=%d\n", size/sizeof(uint8_t) );
	serialPort->print(buffer);
	if (serialPort->find(">"))
	{
		//OK
		serialPort->write(buf,(size/sizeof(uint8_t)));
		serialPort->print("\n");
		return size;
	} else {
		serialPort->println("AT+CIPCLOSE");
		return 0;
	}


}

int ESP8266Client::available() {
	if (sendWaitRespond(AT, "OK", 10000 ))
	{
		return 1;
	}else{
		return 0;
	}

}

/*
 * Received data starts with +IPD
 */
int ESP8266Client::read() {
	serialPort->println("::read");
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
	serialPort->println("::read(uint8_t");
	//return recv(_sock, buf, size);
	return 0;
}

int ESP8266Client::peek() {
	serialPort->println("::peek");
	/*  uint8_t b;
	 // Unlike recv, peek doesn't check to see if there's any data available, so we must
	 if (!available())
	 return -1;
	 ::peek(_sock, &b);
	 return b;*/
	return 0;
}

void ESP8266Client::flush() {
	serialPort->println("::flush");
	while (available())
	read();
}

void ESP8266Client::stop() {
	serialPort->print( AT_DISCONNECT );
}

uint8_t ESP8266Client::connected() {
#ifdef DEBUG_SERIAL
	Serial.println("::connected()");
#endif
	return sendWaitRespond("AT+CIPSTATUS", "STATUS:5", 300 );
}

uint8_t ESP8266Client::status() {
#ifdef DEBUG_SERIAL
	Serial.println("::status()");
#endif
	return sendWaitRespond("AT+CIPSTATUS", "STATUS:5", 300 );
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

	if( !sendWaitRespond("AT", "OK", 300 ) ){
		!sendWaitRespond("AT+RST", "wwww.", 500 );
		if( !sendWaitRespond("AT", "OK", 300 ) ){
			#ifdef DEBUG_SERIAL
			Serial.println("AT FAIL");
			#endif
			return false;
		}

	}
#ifdef DEBUG_SERIAL
		Serial.println("AT OK");
#endif
	serialPort->println("AT+CWQAP");
	delay(500);
	serialPort->println("AT+CWMODE=1");
	delay(500);
	sprintf( buffer, "AT+CWJAP=\"%s\",\"%s\"", ssid, password);

	return sendWaitRespond(buffer, "OK", 10000 );
	/*serialPort->println( buffer );
	delay(2000);
	if(serialPort->find("OK")) {
		return true;
	} else {
		return false;
	}*/
}
String recBuffer;
bool ESP8266Client::sendWaitRespond( char *message, char *response, int msTimeout) {
	bool bussy = false;
	do{
		bussy = false;
		serialPort->println(message);
		serialPort->setTimeout(msTimeout);
		#ifdef DEBUG_SERIAL
			Serial.println(message);
		#endif

		if( serialPort->find(response) ){
			#ifdef DEBUG_SERIAL
					Serial.println("ret true");
					serialPort->flush();
			#endif
				return true;
		}
		if( strstr(serialPort->_receive_buffer, "busy p")){
			bussy =true;
			#ifdef DEBUG_SERIAL
				Serial.println("bussy");
				serialPort->flush();
			#endif
		}
	}while(bussy);
	#ifdef DEBUG_SERIAL
		Serial.println(serialPort->_receive_buffer);
		Serial.println("ret false");
	#endif
	return false;
}
