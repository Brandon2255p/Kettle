#ifndef __SERVER_SENT_EVENT__
#define __SERVER_SENT_EVENT__

class ServerSentEvent
{
public:
    void Handle();
private:
    const char * Header =
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/event-stream\r\n"
    "Connection: Keep-Alive\r\n"
    "Cache-Control: No-Cache\r\n";

    void sendEvent(int id, String type, String content);

}


#endif /* end of include guard: __SERVER */_SENT_EVENT__
