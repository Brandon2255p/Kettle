#define DEBUG_SERIAL sseSerial

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <kettle.h>
#include <serverSentEvent.h>
#include <ESP8266WebServer.h>

const char* ssid = "Router";
const char* password = "cupcak3sinmyExt";
Kettle kettle;
WiFiServer server(80);

void setupHttpServer(){
    server.begin();
}

void OverTempCallback(){
    kettle.StopBoiling();
    sseSerial.print("Temp limit reached");
}

void setup() {
    Serial.begin(9600);
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
    sseSerial.Handle();
    WiFiClient client = server.available();
    if (!client) {
        return;
    }
    //Client has connected
    String request = client.readStringUntil('\r');
    if (request.indexOf("/BOIL") != -1){
        sseSerial.println("Client Requested Boil");
        kettle.StartBoiling();
    }
    else if (request.indexOf("/STOP") != -1){
        sseSerial.println("Client Requested Stop Boil");
        kettle.StopBoiling();
    }
    else if (request.indexOf("/REBOOT") != -1){
        sseSerial.println("Client Requested REBOOT");
        ESP.restart();
    }
    else if (request.indexOf("/stats") != -1){
        Serial.println("Client Requested Stats");
        sseSerial.SetClient(client);
        return;
    }
    String header =  "HTTP/1.1 200 OK\r\n";
    String response =
    "<!DOCTYPE html><html><head><title>Kettle</title>"
    "<script>"
        "function LoadSSE(){"
            "var source = new EventSource(\"/stats\");"
            "source.onmessage = function(event) {"
                "console.log(event);"
                //"document.getElementById(\"result\").innerHTML = event.data + \"<br>\";"
                //"document.getElementById(\"result\").scrollTop += 30;"
            "};"
            "source.addEventListener('message', function(e) {"
                "document.getElementById(\"result\").innerHTML += e.data + \"<br>\";"
                "document.getElementById(\"result\").scrollTop += 30;"
            "}, false);"
            "source.addEventListener('temperature', function(e) {"
                "document.getElementById('temperature').innerHTML = 'Kettle Temperature: ' + e.data + ' C' + \"<br>\";"
            "}, false);"
            "source.addEventListener('open', function(e) {"
                "document.getElementById(\"result\").innerHTML += 'OPENED' + e + \"<br>\";"
            "}, false);"
            "source.addEventListener('error', function(e) {"
              "if (e.readyState == EventSource.CLOSED) {"
              "document.getElementById(\"result\").innerHTML += 'Lost connection';"
              "}"
            "}, false);"
        "}"
        "function CallApi(endpoint) {"
        "var xhttp = new XMLHttpRequest();"
        "xhttp.onreadystatechange = function() {"
            "if (this.readyState == 4 && this.status == 200) {"
               "alert('Doing ' + endpoint);"
            "}};"
        "xhttp.open('GET', endpoint, true);"
        "xhttp.send();}"
    "</script>"
    "</head>"
    "<body onload=\"LoadSSE()\">"
    "<div id='temperature'></div>"
    "<div id='result' style='overflow:auto;width:200px;height:200px;border:solid 1px green;'></div>"
    "<button onclick=\"CallApi('BOIL')\">Boil</button><br><br>"
    "<button onclick=\"CallApi('STOP')\">Stop</button><br><br>"
    "<button onclick=\"CallApi('REBOOT')\">Reboot</button><br><br>"
    "<button onclick=\"LoadSSE()\">Reconnect</button><br><br>"
    "</body>"
    "</html>";
    header += "Content-Type: text/html\r\n";
    header += "Content-Length: " + String(response.length()) + "\r\n";
    client.println(header);
    client.println(response);
    client.println("");
    client.flush();
}
