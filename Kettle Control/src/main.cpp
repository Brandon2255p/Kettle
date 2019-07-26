#define DEBUG_SERIAL sseSerial
#include "ElectrodragonRelay.h"
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <kettle.h>
#include <serverSentEvent.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <LevelSensor.h>
#include <relay.h>
#include "Html.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include <PID_v1.h>
#include "RemoteDebug.h"        //https://github.com/JoaoLopesF/RemoteDebug

RemoteDebug Debug;
void processCmdRemoteDebug();

const char* ssid = "Too Fly for a WiFi";
const char* password = "cupcak3s";
const char* host = "kettle";

ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater(false);

void serialEvent();
void HandleInput();
bool masterOverride = true;
OneWire sensorBus(Pin_DHT);
DallasTemperature sensors(&sensorBus);
DeviceAddress insideThermometer;
double Setpoint, Input, Output;
double consKp=0.2, consKi=2, consKd=0;
PID myPID(&Input, &Output, &Setpoint, consKp, consKi, consKd, DIRECT);
int WindowSize = 500;
unsigned long windowStartTime;
void setup() {
    windowStartTime=millis();
    Serial.begin(9600);
    Serial.println("Booting");
    Serial.println(ssid);
    windowStartTime=millis();
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    while (WiFi.waitForConnectResult() != WL_CONNECTED) {
        Serial.println("Connection Failed! Rebooting...");
        delay(5000);
        ESP.restart();
    }

    MDNS.begin(host);
    httpUpdater.setup(&httpServer);
    httpServer.begin();
    MDNS.addService("http", "tcp", 80);
    MDNS.addService("telnet", "tcp", 23);
    
    Serial.println("Ready");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
	
    Debug.begin(host); // Initialize the WiFi server
	Debug.setResetCmdEnabled(true); // Enable the reset command
	Debug.showProfiler(true); // Profiler (Good to measure times, to optimize codes)
	Debug.showColors(true); // Color
    Debug.setCallBackProjectCmds(&processCmdRemoteDebug);

    sensors.begin();
    if (!sensors.getAddress(insideThermometer, 0)) Serial.println("Unable to find address for Device 0");
    sensors.setResolution(9);
    Setpoint = 30;
    myPID.SetOutputLimits(0, WindowSize);
    myPID.SetSampleTime(WindowSize);
    myPID.SetMode(AUTOMATIC);
    pinMode(Pin_Relay_1, OUTPUT);
}

void loop() {
    httpServer.handleClient();
    MDNS.update();
    Debug.handle();

    sensors.requestTemperatures();
    Input = sensors.getTempC(insideThermometer);
    debugV("%0.2f vs %0.2f",Input, Setpoint);
    if (millis() - windowStartTime > WindowSize)
    {
        auto res = myPID.Compute();
        windowStartTime += WindowSize;
    }
    if(masterOverride)
    {
        debugV("Overrideing");
        digitalWrite(Pin_Relay_1, LOW);
        return;
    }
    if (Output > 2)
    {
        digitalWrite(Pin_Relay_1, HIGH);
    }
    else
    {
        digitalWrite(Pin_Relay_1, LOW);
    }
    debugV("On %d",Output > 1);
}

void processCmdRemoteDebug() {

	String lastCmd = Debug.getLastCommand();

	if (lastCmd.startsWith("off"))
    {
        masterOverride = true;
    }
    if (lastCmd.startsWith("on"))
    {
        masterOverride = false;
    }
    if (lastCmd.startsWith("sp"))
    {
        auto sp=lastCmd.substring(2);
        Setpoint = sp.toDouble();
    }
}