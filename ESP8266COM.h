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
#include "SoftwareSerialLocal.h"

class ESP8266COM {

//Callback with line reaceive
public:
	ESP8266COM( SoftwareSerialLocal *port, void(*)(char*,int*) );
	//Returns a line
	void (*callback)(char*,int*);

	int read();

};

#endif
