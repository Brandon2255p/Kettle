#include "serverSentEvent.h"

ServerSentEvent::ServerSentEvent(int _updatePeriod_ms):
    updatePeriod_ms(_updatePeriod_ms)
{
}

void ServerSentEvent::Handle()
{
    if(streamClient.connected()) {
        unsigned long currentMillis = millis();
        if (currentMillis - previousMillis >= updatePeriod_ms) {
            previousMillis = currentMillis;
            Serial.println("Updating stream client");
            sendEvent("hello");
        }
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
