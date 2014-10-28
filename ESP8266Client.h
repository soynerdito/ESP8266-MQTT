#ifndef esp8266client_h
#define esp8266tclient_h
#include "Arduino.h"	
//#include "Print.h"
#include "Client.h"
#include "IPAddress.h"

class ESP8266Client : public Client {

public:
  ESP8266Client();
  ESP8266Client(uint8_t sock);

  uint8_t status();
  virtual int connect(IPAddress ip, uint16_t port);
  virtual int connect(const char *host, uint16_t port);
  virtual size_t write(uint8_t);
  virtual size_t write(const uint8_t *buf, size_t size);
  virtual int available();
  virtual int read();
  virtual int read(uint8_t *buf, size_t size);
  virtual int peek();
  virtual void flush();
  virtual void stop();
  virtual uint8_t connected();
  virtual operator bool();
  virtual bool operator==(const ESP8266Client&);
  virtual bool operator!=(const ESP8266Client& rhs) { return !this->operator==(rhs); };
  bool connectAP(char *ssid, char *password);

  bool sendWaitRespond(char *message, char *response, int msTimeout);
//  using Print::write;

private:
  static uint16_t _srcport;
  uint8_t _sock;
};

#endif
