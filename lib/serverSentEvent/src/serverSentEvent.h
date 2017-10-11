#ifndef __SERVER_SENT_EVENT__
#define __SERVER_SENT_EVENT__
#include <Arduino.h>
#include <ESP8266WebServer.h>

class ServerSentEvent
{
public:
    ServerSentEvent(int updatePeriod_ms);
    void Handle();
    void SetClient(WiFiClient client);
    void sendEvent(String content, String type = "", String id = "");
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


#endif /* end of include guard: __SERVER _SENT_EVENT__*/
