extern "C" {
#include "string.h"
}

#include "Arduino.h"

#include "ESP8266Client.h"
#include <ESP8266COM.h>

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

#define DEBUG_SERIAL

char *mExpectedResponse = 0;
bool *mReponseFound = 0;
bool setWaitFor(char *expectedResponse, bool *found){
	mExpectedResponse = expectedResponse;
	mReponseFound = found;
}

void callback(void *reference, char *msg, int *len )
{
	if( msg[0] == 13 || msg[0] == 10 ){
		//ignore empty lines
		return;
	}
	ESP8266Client *me = (ESP8266Client*)reference;
	if(mExpectedResponse!= 0 &&  mReponseFound!=0 ){
		//check response
		if(msg!=0 && strstr(msg, mExpectedResponse ) ){
			*mReponseFound = true;
			mExpectedResponse = 0;
			mReponseFound = 0;
			Serial.println("Expected FOUND");
			return;
		}
	}
	//Serial.println("Callback");
	//Serial.println(msg);
	me->parse( msg, len);
	me->available();
}


/*
 * Using a modified SoftwareSerial library to read directly what is on the Serial Buffer
 * Official SoftwareSerial has receive buffer private, modified to use it public
 */
ESP8266Client::ESP8266Client() : _sock(MAX_SOCK_NUM) {
}


int ESP8266Client::connect(const char* host, uint16_t port) {

	// Look up the host first
	// No DNS implementation as of now
#ifdef DEBUG_SERIAL
	Serial.println("::connect(DNS)");
#endif
	return 1;
}
void ESP8266Client::setPort( Stream *port ){
	this->mESPModule.initialize(this, port, callback  );
}

int ESP8266Client::connect(IPAddress ip, uint16_t port) {
#ifdef DEBUG_SERIAL
	Serial.println("::connect(uint8_t)");
#endif
	//Close anny connection
	//serialPort->println("AT+CIPCLOSE");
	mESPModule.write("AT+CIPCLOSE");
	delay(20);
	flush();
	uint8_t* rawAdress = rawIPAddress(ip);
	sprintf( buffer, "AT+CIPSTART=\"TCP\",\"%d.%d.%d.%d\",%d", rawAdress[0], rawAdress[1], rawAdress[2], rawAdress[3], port);
#ifdef DEBUG_SERIAL
	Serial.println(buffer);
#endif
	//serialPort->println(buffer);
	//serialPort->setTimeout(5000);
	if( sendWaitRespond(buffer,"OK",5000) ){
#ifdef DEBUG_SERIAL
	Serial.println("connect true");
#endif
		return 1;
	} else {
#ifdef DEBUG_SERIAL
	Serial.println("false");
#endif
		return 0;
	}
}
unsigned long start;
size_t ESP8266Client::write(uint8_t b) {
#ifdef DEBUG_SERIAL
	Serial.println("::write(uint8_t)");
#endif
	start = millis();
	while(!sendWaitRespond("AT","OK",10000) ){
		if( (start -millis()) > 5000 ){
			Serial.println("FAIL TO WRITE");
			return 0;
		}
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
		mESPModule.write(b);
		mESPModule.write('\n');
		//serialPort->println();
		return sizeof(uint8_t);
	}else
	{
		//serialPort->println("AT+CIPCLOSE");
		mESPModule.write("AT+CIPCLOSE");
		return 0;
	}
}

size_t ESP8266Client::write(const uint8_t *buf, size_t size) {
#ifdef DEBUG_SERIAL
	Serial.println("::write()");
#endif
	start = millis();
	while(!sendWaitRespond("AT","OK",10000) )
	{
		if( (start -millis()) > 5000 ){
			Serial.println("FAIL TO WRITE");
			return 0;
		}
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
		mESPModule.write(buf, size);
		mESPModule.write('\n');
		//serialPort->write(buf,(sizeof(uint8_t) * size));
		//serialPort->print("\n");
		//serialPort->flush();
		return size;
	} else {
		mESPModule.write("AT+CIPCLOSE");
		//serialPort->println("AT+CIPCLOSE");
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
	/*if( buffPos < buffSize ){
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

    if( serialPort->find("+IPD,") ){
    	buffSize=0;
    	Serial.println("Found IPD");
    	char tmp;
		while( serialPort->peek() != ':' ){
			buffSize = buffSize * 10;
			tmp = serialPort->read();
			Serial.print(tmp);
			buffSize += (tmp - 48);

		}
		Serial.print("Message Size = ");
		Serial.println(buffSize);
    	//Move till the :
    	serialPort->find(":");
    	buffPos=0;
    	while( buffPos <  buffSize ){
    		inBuffer[buffPos] = char(serialPort->read());
			Serial.print(char(inBuffer[buffPos]));
			buffPos++;
			//wait for next character...
			while( !serialPort->available() ){
				Serial.println("reading...");
				delay(10);
			}
    	}
    	buffPos = 0;
#ifdef DEBUG_SERIAL
    	if( buffSize > 0 ){
    		Serial.println("data in");
    	}
#endif

			//inBuffer[buffPos] = 0;
			buffPos = 0;
			flush();
			//forget about anything else
			//flush();
			//return read();
			buffPos++;
			return inBuffer[0];
	}else{
		Serial.println("no IPD");
	}

	Serial.println("ret -1");*/
	return -1;
}

int ESP8266Client::available() {
	mESPModule.read();
	if( buffSize > 0 ) return true;
	if( read() < 0 ) return false;
	Serial.println( "avilable Read");
	if( buffPos> 0 ){
		Serial.println( "buffPos >0");
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
	return -1;//serialPort->read();
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
	return 0;//serialPort->peek();
}

void ESP8266Client::flush() {
#ifdef DEBUG_SERIAL
	Serial.println("::flush()");
#endif
	mESPModule.flush();
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
		Serial.println("Something found");
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

	if( !sendWaitRespond("AT", "OK", 10000 ) ){
		!sendWaitRespond("AT+RST", "wwww.", 10000 );
		if( !sendWaitRespond("AT", "OK", 10000 ) ){
			#ifdef DEBUG_SERIAL
			Serial.println("AT FAIL");
			#endif
			return false;
		}
	}
#ifdef DEBUG_SERIAL
	Serial.println("AT OK");
#endif
	flush();
	mESPModule.write("AT+CWQAP");
	delay(500);
	mESPModule.write("AT+CWMODE=1");
	delay(500);
	flush();
	sprintf( buffer, "AT+CWJAP=\"%s\",\"%s\"", ssid, password);

	return sendWaitRespond(buffer, "OK", 10000 );
}

void ESP8266Client::parse( char *msg, int *len ){
	//Parse this message
	mLastResponse = msg;
}

bool timeout(unsigned long uStart, int timeout ){
	return ((millis() - uStart ) > timeout);
}

bool responseFound = false;
unsigned long startTime;
bool ESP8266Client::singleSendWait(char *message, char *response, int mstimeout){
	responseFound = false;
	setWaitFor(response, &responseFound );
	startTime = millis();
	while(!responseFound){
		mESPModule.write(message);
		mESPModule.read();
		delay(100);
		if( timeout(startTime, mstimeout)){
			Serial.println("Timeout");
			return false;
		}
	}
	return true;
}

bool ESP8266Client::sendWaitRespond( char *message, char *response, int msTimeout ){
	//Flush port
	//Set Expected response
	//Post Message
	//msTimeout = 20000;
	mESPModule.flush();
	clearLast();
	//Check if device ready for use
	Serial.println("checking AT for");
	Serial.println(message);
	Serial.print("Response ( ");
	Serial.print(response);
	Serial.println(" )");

	if( !singleSendWait("AT","OK", msTimeout) ){
		Serial.println("AT FAIL");
		return false;
	}
	Serial.println("AT Success");

	bool bussy = true;
	startTime = millis();
	for( int i=0;i< 5; i++) {
	//do{
		if( singleSendWait(message,response, msTimeout) ){
			Serial.println("ret true");
			return true;
		}
		//mESPModule.flush();
		//mESPModule.write(message);
		delay(50);
	}//while(bussy);
	return false;

}

