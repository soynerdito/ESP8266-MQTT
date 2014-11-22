/*
  esp8266com.h - Class in charge of parsing data from the ESP8266
  A callback function is called on each entire line read
  There is a current buffer maximun size for line

  This library is free software; blah blah blah Public Domain.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

*/

#ifndef esp8266com_h
#define esp8266com_h
#include "Arduino.h"	
#include "Client.h"
#include "IPAddress.h"

class ESP8266COM {

//Callback with line reaceive
public:
	ESP8266COM(void *object, Stream *port, void(*)(void*,char*,int*) );
	ESP8266COM( );
	void initialize(void *object, Stream *port, void(*callback)(void*,char*,int*)  );
	int write(char *message);
	void write(uint8_t b) { mSerialPort->write(b); }
	void write(const uint8_t *buf, size_t size){
		mSerialPort->write(buf,(sizeof(uint8_t) * size));
	}
	//Returns a line
	void (*callback)(void*,char*,int*);
	void flush(){ mSerialPort->flush(); }

	int read();

private:
	void *mReference;
	Stream *mSerialPort;

};

#endif
