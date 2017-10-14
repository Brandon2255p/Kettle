#ifndef __SERVER_SENT_EVENT__
#define __SERVER_SENT_EVENT__
#include <Arduino.h>
#include <ESP8266WebServer.h>


class ServerSentEvent : public Print
{
public:
    ServerSentEvent(int updatePeriod_ms = 1000);
    void Handle();
    void SetClient(WiFiClient client);
    void sendEvent(String content, String type = "", String id = "");
    virtual size_t write(uint8_t);
    virtual size_t write(const uint8_t *buffer, size_t size);
private:
    const char * Header =
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/event-stream\r\n"
    "Connection: Keep-Alive\r\n"
    "Cache-Control: No-Cache\r\n";

    WiFiClient streamClient;
    long previousMillis;
    int updatePeriod_ms;
};

extern ServerSentEvent sseSerial;

#endif /* end of include guard: __SERVER _SENT_EVENT__*/
