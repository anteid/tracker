#ifndef MessagesManager_h
#define MessagesManager_h

#include <Arduino.h>
#include <WiFiUdp.h>

class MessagesManager {
  public:
  MessagesManager();
  void init(const char host[],const uint32_t port);
  void send(uint32_t msgid, const char *fmt, ...);
private:
  char _host[15];
  uint32_t _port;
  WiFiUDP client;
};

#endif
