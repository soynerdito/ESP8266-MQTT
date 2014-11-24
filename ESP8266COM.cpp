extern "C" {
#include "string.h"
}

#include "Arduino.h"

#include "ESP8266COM.h"

#define MAX_BUFF_SIZE 200

char mBufferLine1[MAX_BUFF_SIZE];

ESP8266COM::ESP8266COM(void *object, SoftwareSerialLocal *port, void(*callback)(void*, char*,int*) )  {
	this->initialize(object, port, callback );
}

ESP8266COM::ESP8266COM( )  {
	mSerialPort = 0;
	this->callback = 0;
}

void ESP8266COM::initialize(void *object, SoftwareSerialLocal *port, void(*callback)(void*, char*, int* )  )  {
	mSerialPort = port;
	this->callback = callback;
	this->mReference = object;
}

int ESP8266COM::write(char *message){
	Serial.print( "ESP8266COM::write ");
	Serial.println( message );
	mSerialPort->println(message);
	mSerialPort->available();
}

int ESP8266COM::print(uint8_t b){
	mSerialPort->write( b );
}

int readPos = 0;
char newByte;
int ESP8266COM::read() {
	//zero out buffer
	if( readPos == 0 ){
		memset(mBufferLine1, 0, sizeof(mBufferLine1));
	}
	//memset(mBufferLine1, 0, sizeof(mBufferLine1));
	//readPos = 0;
	//We ready to have some readings
	//mSerialPort->listen();
	int readCount;
	while( mSerialPort->available() ){
		newByte = mSerialPort->read();

		if( newByte == 10 || newByte == 13 ){
			if( readPos> 0 ){
				this->callback(this->mReference, mBufferLine1, &readPos );
				readCount = readPos;
				readPos = 0;
			}
			return readCount;
		}
		//If longer data will be truncated - deal with it
		if( readPos < MAX_BUFF_SIZE ){
			mBufferLine1[readPos]= newByte;
			readPos++;
		}
		/*if( newByte == '>' ){
			this->callback(this->mReference, mBufferLine1, &readPos );
			return 1;
		}*/
		//delay(10);
		//mSerialPort->listen();
	}

	return readPos;
}
