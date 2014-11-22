extern "C" {
#include "string.h"
}

#include "Arduino.h"

#include "ESP8266COM.h"




#define MAX_BUFF_SIZE 200

char mBufferLine1[MAX_BUFF_SIZE];

ESP8266COM::ESP8266COM(void *object, Stream *port, void(*callback)(void*, char*,int*) )  {
	this->initialize(object, port, callback );
}

ESP8266COM::ESP8266COM( )  {
	mSerialPort = 0;
	this->callback = 0;
}

void ESP8266COM::initialize(void *object, Stream *port, void(*callback)(void*, char*, int* )  )  {
	mSerialPort = port;
	this->callback = callback;
	this->mReference = object;
}

int ESP8266COM::write(char *message){
	mSerialPort->println(message);
	delay(10);
}

int readPos = 0;
char newByte;
int ESP8266COM::read() {
	//zero out buffer
	memset(mBufferLine1, 0, sizeof(mBufferLine1));
	readPos = 0;
	//We ready to have some readings
	//mSerialPort->listen();
	while( mSerialPort->available() ){
		newByte = mSerialPort->read();
		if( newByte == 10 || newByte == 13 ){
			if( readPos> 0 ){
				this->callback(this->mReference, mBufferLine1, &readPos );
			}
			return readPos;
		}
		//If longer data will be truncated - deal with it
		if( readPos < MAX_BUFF_SIZE ){
			mBufferLine1[readPos]= newByte;
			readPos++;
		}
		//mSerialPort->listen();
	}

	return readPos;
}
