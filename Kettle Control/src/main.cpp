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
#include <OneWire.h>
#include <DallasTemperature.h>
#include <PID_v1.h>

const char* ssid = "Too Fly for a WiFi";
const char* password = "cupcak3s";

WiFiServer server(80);


void serialEvent();
void HandleInput();

void setupHttpServer(){
    server.begin();
}


OneWire sensorBus(Pin_DHT);
DallasTemperature sensors(&sensorBus);
DeviceAddress insideThermometer;
double Setpoint, Input, Output;
double consKp=2, consKi=0, consKd=0;
PID myPID(&Input, &Output, &Setpoint, consKp, consKi, consKd, DIRECT);
int WindowSize = 1000;
unsigned long windowStartTime;
void setup() {
    Serial.println (ESP.getSdkVersion());
    windowStartTime=millis();
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

    setupHttpServer();

    sensors.begin();
    if (!sensors.getAddress(insideThermometer, 0)) Serial.println("Unable to find address for Device 0");
    sensors.setResolution(10);
    Setpoint = 80;
    myPID.SetOutputLimits(0, WindowSize);
    myPID.SetSampleTime(WindowSize);
    myPID.SetMode(AUTOMATIC);
    pinMode(Pin_Relay_1, OUTPUT);
}

void loop() {
    ArduinoOTA.handle();
    sensors.requestTemperatures();
    Input = sensors.getTempC(insideThermometer);
    Serial.println(Input);
    if (millis() - windowStartTime > WindowSize)
    {
        Serial.println("Computing");
        auto res = myPID.Compute();
        Serial.println("res");
        Serial.println(res);

        windowStartTime += WindowSize;
    }
    Serial.println("Output");
    Serial.println(Output);
    if (Output > 1)
    {
        Serial.println("ON");
        digitalWrite(Pin_Relay_1, HIGH);
    }
    else
    {
        Serial.println("OFF");
        digitalWrite(Pin_Relay_1, LOW);
    } 
}

