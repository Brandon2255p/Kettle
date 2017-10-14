#include "serverSentEvent.h"

ServerSentEvent sseSerial(1000);

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
            // Serial.println("Updating stream client");
            // sendEvent("", "keep-alive");
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

size_t ServerSentEvent::write(uint8_t)
{
    sendEvent("error_writing");
}

size_t ServerSentEvent::write(const uint8_t *buffer, size_t size)
{
    String toSend = String((const char*)buffer);
    sendEvent(toSend);
}
