#include <MessagesManager.h>
#include <libpomp.h>
#include <pomp_priv.h>

MessagesManager::MessagesManager(){
  strcpy(_host,"0.0.0.0");
  _port = 0;
}

void MessagesManager::init(const char* host, const uint32_t port)
{
  strcpy(_host,host);
  _port = port;
  client.begin(_port);
}

void MessagesManager::send(uint32_t msgid, const char *fmt, ...)
{
  struct pomp_msg msg = POMP_MSG_INITIALIZER;
  struct pomp_encoder enc = POMP_ENCODER_INITIALIZER;

  va_list args;
  va_start(args, fmt);

  pomp_msg_init(&msg,msgid);
  pomp_encoder_init(&enc,&msg);
  pomp_encoder_writev(&enc,fmt,args);
  pomp_msg_finish(&msg);

  const void* cdata;
  size_t len;
  struct pomp_buffer* buf;

  buf = pomp_msg_get_buffer(&msg);
  pomp_buffer_get_cdata(buf,&cdata,&len,NULL);

  client.beginPacket(_host,_port);
  client.write((const uint8_t *)cdata,len);
  client.endPacket();

  va_end(args);
  pomp_msg_clear(&msg);
}
