#define DEBUG_SERIAL sseSerial
#include "ElectrodragonRelay.h"
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <kettle.h>
#include <serverSentEvent.h>
#include <ESP8266WebServer.h>
#include <LevelSensor.h>
#include <relay.h>
#include "Html.h"

const char* ssid = "Router";
const char* password = "cupcak3sinmyExt";
Kettle kettle;
WiFiServer server(80);
// LevelSensor waterLevelSensor;
Relay WaterInputPump(Pin_Relay_2);

String command = "";         // a String to hold incoming data
bool commandComplete = false;  // whether the string is complete
void serialEvent();
void HandleInput();

void setupHttpServer(){
    server.begin();
}

void OverTempCallback(){
    kettle.StopBoiling();
    sseSerial.print("Temp limit reached");
}

// void WaterLevelReachedCallback()
// {
//     sseSerial.print("Water level reached");
// }

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
    ArduinoOTA.setHostname("Kettle");
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

    // waterLevelSensor.Setup(Pin_Button_0, WaterLevelReachedCallback);
    setupHttpServer();
    ESP.wdtDisable();
    ESP.wdtEnable(WDTO_8S);
}

void loop() {
    ArduinoOTA.handle();
    kettle.Handle();
    sseSerial.Handle();
    serialEvent();
    HandleInput();
    if(kettle.SignalBoilComplete())
    {
        sseSerial.println("Sending pour command");
        Serial.println("cmdpour");
    }
    // waterLevelSensor.Handle();

    WiFiClient client = server.available();
    if (!client) {
        return;
    }
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
    else if (request.indexOf("/DROP") != -1){
        Serial.println("cmddrop");
    }
    else if (request.indexOf("/PUMP") != -1){
        Serial.println("cmdpump");
    }
    else if (request.indexOf("/POUR") != -1){
        Serial.println("cmdpour");
    }
    else if (request.indexOf("/RESET") != -1){
        Serial.println("cmdx");
    }
    else if (request.indexOf("/stats") != -1){
        Serial.println("Client Requested Stats");
        sseSerial.SetClient(client);
        return;
    }

    client.println(header + "Content-Length: " + String(response.length()) + "\r\n");
    client.println(response);
    client.println("");
    client.flush();
}


void serialEvent() {
  while (Serial.available()) {
    char inChar = (char)Serial.read();
    if (inChar == '\n') {
        commandComplete = true;
        return;
    }
    command += inChar;
  }
}

void HandleInput()
{
    if (commandComplete) {
        if(command.indexOf("cmdboil") !=  -1)
        {
            sseSerial.println("BOIL SIGNAL");
            kettle.StartBoiling();
        }
        else
        {
            sseSerial.println(command);
        }
        commandComplete = false;
        command = "";
    }
}
