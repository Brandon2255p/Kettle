#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <kettle.h>
#include <serverSentEvent.h>
#include <ESP8266WebServer.h>
#include <KettleTempControl.h>

const char* ssid = "Router";
const char* password = "cupcak3sinmyExt";
Kettle kettle;
WiFiServer server(80);
ServerSentEvent sse(2000);
KettleTempControl kettleTempControl;

void setupHttpServer(){
    server.begin();
}

void setup() {
  Serial.begin(115200);
  Serial.println("Booting");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }
  ArduinoOTA.setHostname("kettle");
  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  setupHttpServer();
  ESP.wdtDisable();
  ESP.wdtEnable(WDTO_8S);
}

void loop() {
    ArduinoOTA.handle();
    kettle.Handle();
    sse.Handle();
    WiFiClient client = server.available();
    if (!client) {
        return;
    }
    //Client has connected
    String request = client.readStringUntil('\r');
    if (request.indexOf("/BOIL") != -1){
        Serial.println("Client Requested Boil");
        kettle.StartBoiling();
    }
    else if (request.indexOf("/STOP") != -1){
        Serial.println("Client Requested Stop Boil");
        kettle.StopBoiling();
    }
    else if (request.indexOf("/stats") != -1){
        Serial.println("Client Requested Stats");
        sse.SetClient(client);
        return;
    }
    String header =  "HTTP/1.1 200 OK\r\n";
    String response =
    "<!DOCTYPE html><html><head><title>Kettle</title>"
    "<script>"
        "function LoadSSE(){"
            "var source = new EventSource(\"/stats\");"
            "source.onmessage = function(event) {"
                "document.getElementById(\"result\").innerHTML += event.data + \"<br>\";"
            "};"
            "source.addEventListener('open', function(e) {"
                "document.getElementById(\"result\").innerHTML += 'OPENED' + e + \"<br>\";"
            "}, false);"
            "source.addEventListener('error', function(e) {"
              "if (e.readyState == EventSource.CLOSED) {"
              "document.getElementById(\"result\").innerHTML += 'CLOSED' + e + \"<br>\";"
              "}"
            "}, false);"
        "}"
    "</script>"
    "</head>"
    "<body id=\"result\" onload=\"LoadSSE()\">"
    "The content of the document......"
    "</body>"
    "</html>";
    header += "Content-Type: text/html\r\n";
    header += "Content-Length: " + String(response.length()) + "\r\n";
    client.println(header);
    client.println(response);
    client.println("");
    client.flush();
}
