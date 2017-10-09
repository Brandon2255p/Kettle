#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <kettle.h>
#include <ESP8266WebServer.h>

const char* ssid = "Router";
const char* password = "cupcak3sinmyExt";
Kettle kettle;
ESP8266WebServer server(80);

void handleRoot() {
    server.send(200, "text/plain", "hello from kettle!");
}
void handleNotFound(){
    String message = "File Not Found\n\n";
    message += "URI: ";
    message += server.uri();
    message += "\nMethod: ";
    message += (server.method() == HTTP_GET)?"GET":"POST";
    message += "\nArguments: ";
    message += server.args();
    message += "\n";
    for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
    }
    server.send(404, "text/plain", message);
}
void setupHttpServer(){
    server.on("/", handleRoot);

    server.onNotFound(handleNotFound);

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
    server.handleClient();

    // WiFiClient client = server.available();
    // if (!client) {
    //     return;
    // }
    // //Client has connected
    // String request = client.readStringUntil('\r');
    // if (request.indexOf("/BOIL") != -1){
    //     Serial.println("Client Requested Boil");
    //     kettle.StartBoiling();
    // }
    // else if (request.indexOf("/STOP") != -1){
    //     Serial.println("Client Requested Stop Boil");
    //     kettle.StopBoiling();
    // }
    // client.println("HTTP/1.1\r\n200 OK");
    // client.println("<!DOCTYPE html><html><head><title>Kettle</title>");
    // client.println("<script>");
    //     client.println("function LoadSSE(){");
    //         client.println("var source = new EventSource(\"/stats\");");
    //         client.println("source.onmessage = function(event) {");
    //             client.println("document.getElementById(\"result\").innerHTML += event.data + \"<br>\";");
    //         client.println("};");
    //     client.println("}");
    // client.println("</script>");
    // client.println("</head>");
    // client.println("<body id=\"result\" onload=\"LoadSSE()\">");
    // client.println("The content of the document......");
    // client.println("</body>");
    // client.println("</html>");
    // client.flush();
}
