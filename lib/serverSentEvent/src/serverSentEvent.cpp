#include "serverSentEvent.h"
void ServerSentEvent::Handle()
{
    if(streamClient.connected()) {
        Serial.println("Found stream client");
        sendEvent("hello");
        // streamClient.print("data: continued event\n\n");
        streamClient.flush();
    }
}

void ServerSentEvent::sendEvent(String content, String type, String id)
{
    String sending = "id: " + id  + "\n" + "event: " +  type + "\n" + "data: " + content + "\n\n";
    streamClient.print(sending);
}

void ServerSentEvent::SetClient(WiFiClient client)
{
    streamClient = client;
    streamClient.println(Header);
    sendEvent("");
}
