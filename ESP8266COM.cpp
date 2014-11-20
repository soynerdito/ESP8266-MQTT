extern "C" {
#include "string.h"
}

#include "Arduino.h"

#include "ESP8266COM.h"

SoftwareSerialLocal *mSerialPort;

#define MAX_BUFF_SIZE 200

char mBufferLine1[MAX_BUFF_SIZE];

ESP8266COM::ESP8266COM( SoftwareSerialLocal *port, void(*callback)(char*,int*) )  {
	mSerialPort = port;
	this->callback = callback;
}

int readPos = 0;
char newByte;
int ESP8266COM::read() {
	//zero out buffer
	memset(mBufferLine1, '\0', sizeof(mBufferLine1));
	readPos = 0;
	//We ready to have some readings
	mSerialPort->listen();
	while( mSerialPort->available() ){
		newByte = mSerialPort->read();
		if( newByte == 10 || newByte == 13 ){
			if( readPos> 0 ){
				this->callback( mBufferLine1, &readPos );
			}
			return readPos;
		}
		//If longer data will be truncated - deal with it
		if( readPos < MAX_BUFF_SIZE ){
			mBufferLine1[readPos]= newByte;
			readPos++;
		}
		mSerialPort->listen();
	}

	return readPos;
}
