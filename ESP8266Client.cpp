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
SoftwareSerialLocal *serialPort=0;

//#define DEBUG_SERIAL
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
#ifdef DEBUG_SERIAL
	Serial.println("::connect(DNS)");
#endif
	return 1;
}
void ESP8266Client::setSerial( SoftwareSerialLocal *port ){
	serialPort = port;
}

int ESP8266Client::connect(IPAddress ip, uint16_t port) {
#ifdef DEBUG_SERIAL
	Serial.println("::connect(uint8_t)");
#endif
	//Close anny connection
	serialPort->println("AT+CIPCLOSE");
	delay(20);

	uint8_t* rawAdress = rawIPAddress(ip);
	sprintf( buffer, "AT+CIPSTART=\"TCP\",\"%d.%d.%d.%d\",%d", rawAdress[0], rawAdress[1], rawAdress[2], rawAdress[3], port);
#ifdef DEBUG_SERIAL
	Serial.println(buffer);
#endif
	//serialPort->println(buffer);
	//serialPort->setTimeout(5000);
	if( sendWaitRespond(buffer,"OK",5000) ){
#ifdef DEBUG_SERIAL
	Serial.println("true");
#endif
		return 1;
	} else {
#ifdef DEBUG_SERIAL
	Serial.println("false");
#endif
		return 0;
	}
}

size_t ESP8266Client::write(uint8_t b) {
#ifdef DEBUG_SERIAL
	Serial.println("::write(uint8_t)");
#endif
	while(!sendWaitRespond("AT","OK",10000) ){

	}
#ifdef DEBUG_SERIAL
	Serial.println("ready to write byte");
#endif

	//serialPort->print("AT+CIPSEND=1");
	//if (serialPort->find(">"))
	if(sendWaitRespond("AT+CIPSEND=1",">",10000) )
	{
#ifdef DEBUG_SERIAL
	Serial.println("byte sent");
#endif
		//OK
		serialPort->write(b);
		serialPort->println();
		return sizeof(uint8_t);
	}else
	{
		serialPort->println("AT+CIPCLOSE");
		return 0;
	}
}

size_t ESP8266Client::write(const uint8_t *buf, size_t size) {
#ifdef DEBUG_SERIAL
	Serial.println("::write()");
#endif

	while(!sendWaitRespond("AT","OK",10000) ){
		}
	#ifdef DEBUG_SERIAL
		Serial.println("ready to write bytes");
	#endif
	//serialPort->write(buf,(size/sizeof(uint8_t)));
	sprintf( buffer, "AT+CIPSEND=%d", size/sizeof(uint8_t) );
	if( sendWaitRespond(buffer,">",5000) ) {
#ifdef DEBUG_SERIAL
	Serial.println("writing");
#endif
		//OK
		serialPort->write(buf,(sizeof(uint8_t) * size));
		serialPort->print("\n");
		serialPort->flush();
		return size;
	} else {
		serialPort->println("AT+CIPCLOSE");
		return 0;
	}


}


/*
 * Received data starts with +IPD
 * Returned data must be stripped from +IPD
 * After that comes the amount of byte received
 * On the next line then is the actual data
 */

//Skip chars
//Sample +IPD,0,4:df
char skip[] ={ '+','I','P','D' };

char inBuffer[255];
int buffPos=0;
int buffSize=0;
int ESP8266Client::read( ) {
	if( buffPos < buffSize ){
		//Serial.print("From Buffeer ");
		char retValue = inBuffer[buffPos];
		//Serial.print(retValue);
		buffPos++;
		return retValue;
	}
	//Serial.println("::read");
	buffPos = 0;
	buffSize = 0;
	if( !serialPort->available() ){
		return -1;
	}
	//memset(inBuffer, '\n', sizeof(inBuffer));
	//Skip line feeds
	//Serial.println("to peek");
	/*while( serialPort->peek() == 10
			|| serialPort->peek() == 13){
		serialPort->read();
	}*/
	//Skip +IPDXX:
	//Serial.println("skiping");
	/*if( serialPort->peek() == skip[0] ){
		if( serialPort->read() == skip[0] &&
			serialPort->read() == skip[1] &&
			serialPort->read() == skip[2] &&
			serialPort->read() == skip[3] ){*/

    if( serialPort->find("+IPD,") ){
    	buffSize=0;
#ifdef DEBUG_SERIAL
    	Serial.println("Found IPD");
#endif
    	char tmp;
		while( serialPort->peek() != ':' ){
			tmp = serialPort->read();
			if( tmp <48 || tmp > 57){ continue; }
			buffSize = buffSize * 10;
#ifdef DEBUG_SERIAL
			Serial.print(tmp);
#endif
			buffSize += (tmp - 48);

		}
		if( buffSize< 0){
#ifdef DEBUG_SERIAL
			//This means that the parsing /reading of port is incrrectly done
			Serial.println("Error on Message Size");
#endif
			return -1;
		}
#ifdef DEBUG_SERIAL
		Serial.print("Message Size = ");
		Serial.println(buffSize);
#endif
    	//Move till the :
    	serialPort->find(":");
    	buffPos=0;
    	while( buffPos <  buffSize ){
    		inBuffer[buffPos] = char(serialPort->read());
#ifdef DEBUG_SERIAL
			Serial.print(char(inBuffer[buffPos]));
#endif
			buffPos++;
			//wait for next character...
			while( buffPos <  buffSize && !serialPort->available() ){
#ifdef DEBUG_SERIAL
				Serial.println("reading...");
#endif
				delay(10);
			}
    	}
    	buffPos = 0;
#ifdef DEBUG_SERIAL
    	if( buffSize > 0 ){
    		Serial.println("data in");
    	}
#endif
    	/*while( serialPort->peek() != 10 &&
    			serialPort->peek() != 13 ){
    		inBuffer[buffSize] = char(serialPort->read());
    		Serial.print(char(inBuffer[buffSize]));
    		buffSize++;
    	}
    	Serial.print("Buffer Size = ");
    	Serial.println(buffSize);*/
    	//inBuffer[buffSize] = 13;
    	//buffSize++;
    		/*char tmp;
			while( serialPort->peek() != ':' ){
				buffSize * 10;
				tmp = serialPort->read();
				Serial.print(tmp);
				buffSize += (tmp - 48);
				//buffSize += (char(serialPort->read()) - 48);
				#ifdef DEBUG_SERIAL
					Serial.println(".");
				#endif
				Serial.print(buffSize);
			}
			//Skip next char
			Serial.print(char(serialPort->read()));
			Serial.println("Message Size");
			Serial.println(buffSize);
			bool reading = true;
			buffPos=0;
			while(buffPos < buffSize ){
				inBuffer[buffPos] = serialPort->read();
				Serial.print(char(inBuffer[buffPos]));
				buffPos++;
			}*/
			//inBuffer[buffPos] = 0;
			buffPos = 0;
			flush();
			//forget about anything else
			//flush();
			//return read();
			buffPos++;
			return inBuffer[0];
	}
#ifdef DEBUG_SERIAL
    else{
		Serial.println("no IPD");
	}
    Serial.println("ret -1");
#endif

	return -1;
}

int ESP8266Client::available() {
	if( buffSize > 0 ) return true;
	if( read() < 0 ) return false;
#ifdef DEBUG_SERIAL
	Serial.println( "avilable Read");
#endif
	if( buffPos> 0 ){
#ifdef DEBUG_SERIAL
		Serial.println( "buffPos >0");
#endif
		buffPos--;
		return true;
	}
	//return serialPort->available();
	return false;
}

int ESP8266Client::read(uint8_t *buf, size_t size) {
#ifdef DEBUG_SERIAL
	Serial.println("NOT IMPLEMENTED ::read(uint8)");
#endif
	return serialPort->read();
}

int ESP8266Client::peek() {
#ifdef DEBUG_SERIAL
	Serial.println("::peek()");
#endif
	//serialPort->println("::peek");
	/*  uint8_t b;
	 // Unlike recv, peek doesn't check to see if there's any data available, so we must
	 if (!available())
	 return -1;
	 ::peek(_sock, &b);
	 return b;*/
	return serialPort->peek();
}

void ESP8266Client::flush() {
#ifdef DEBUG_SERIAL
	Serial.println("::flush()");
#endif
	serialPort->flush();
}

void ESP8266Client::stop() {
#ifdef DEBUG_SERIAL
	Serial.println("::stop()");
#endif
	//sendWaitRespond( "AT+CIPCLOSE", "OK", 1500);
}

uint8_t ESP8266Client::connected() {
#ifdef DEBUG_SERIAL
	Serial.println("::connected()");
#endif
	//If Unlink is there then this has been disconnected, no need to continue
	//Yes if you send the word Unlink in your package it might get all messed up
	//return sendWaitRespond("AT+CIPSTATUS", "STATUS:3", 500 );
	read();
	if( buffSize> 0 && buffPos > 0 ){
#ifdef DEBUG_SERIAL
		Serial.println("Something found");
#endif
		buffPos--;
	}
	//check AT first
	//sendWaitRespond("AT","OK", 500 );
	bool connected = sendWaitRespond("AT+CIPSTATUS", "TCP", 1500 );
	/*if( strstr(serialPort->_receive_buffer, "Unlink")){
		serialPort->flush();
		connected =  false;
		Serial.println("Not Connected");
	}*/
	return connected;
	//return sendWaitRespond("AT+CIPSTATUS", "TCP", 1500 );
}

uint8_t ESP8266Client::status() {
#ifdef DEBUG_SERIAL
	Serial.println("::status()");
#endif
	return sendWaitRespond("AT+CIPSTATUS", "STATUS:5", 2500 );
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
bool tryATOK() {
	serialPort->setTimeout(1500);
	serialPort->println("AT");
	if( serialPort->find("OK") ){
		return true;
	}
	return false;
}

String recBuffer;
bool ESP8266Client::sendWaitRespond( char *message, char *response, int msTimeout) {
	bool bussy = false;
	serialPort->flush();
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
			#endif
				serialPort->flush();
				return true;
		}
		if( strstr(serialPort->_receive_buffer, "busy")){
			bussy =true;
			#ifdef DEBUG_SERIAL
				Serial.println("bussy");
				Serial.println(serialPort->_receive_buffer);
			#endif
			serialPort->flush();
			while(!tryATOK()){
				//Serial.println("trying AT");
			}
		}
		delay(50);
	}while(bussy);
	#ifdef DEBUG_SERIAL
		Serial.println(serialPort->_receive_buffer);
		//Serial.println("ret false");
	#endif
	return false;
}
