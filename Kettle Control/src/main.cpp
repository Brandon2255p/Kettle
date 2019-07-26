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
uint pwmDutyOn, pwmDutyOff = 2000;
double Setpoint, CurrentTemp, PreviousTemp, Output;
double consKp=0.05, consKi=0, consKd=1;
PID myPID(&CurrentTemp, &Output, &Setpoint, consKp, consKi, consKd, DIRECT);
uint WindowSize = 500;
bool bIsOn = false;
unsigned long windowStartTime, pwm_time;
void setup() {
    pwm_time = millis();
    Serial.begin(9600);
    Serial.println("Booting");
    Serial.println(ssid);
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
    Setpoint = 110;
    myPID.SetOutputLimits(0, 250);
    myPID.SetSampleTime(5000);
    myPID.SetMode(AUTOMATIC);
    pinMode(Pin_Relay_1, OUTPUT);
    pwmDutyOn = 100;

    httpServer.on("/", HTTP_GET, [&](){
      httpServer.send(200, "text/html", "<html><head></head><body><a href='/update'>update</a></body></html>");
    });
    windowStartTime=millis();
}

bool handleSafetyFailure()
{
    if(CurrentTemp > 127)
    {
        CurrentTemp = 127;
        debugE("Overtemp!!!");
    }
    if(CurrentTemp < 5)
    {
        masterOverride = true;
        debugE("undertemp!!!");
    }
    if(CurrentTemp - PreviousTemp > 30)
    {
        masterOverride = true;
        debugE("Large Temp Diff!!!");
    }
    if(masterOverride)
    {
        digitalWrite(Pin_Relay_1, LOW);
        debugW("Overrideing");
        return true;
    }
    return false;
}

void loop() {
    httpServer.handleClient();
    MDNS.update();
    Debug.handle();
    if(millis() - windowStartTime >  WindowSize)
    {
        windowStartTime=millis();
        sensors.requestTemperatures();
        PreviousTemp = CurrentTemp;
        CurrentTemp = sensors.getTempC(insideThermometer);
        CurrentTemp = CurrentTemp > 127 ? 127.5 : CurrentTemp;
        debugI("%0.2f# was %0.2f now %0.2f", Setpoint, PreviousTemp, CurrentTemp);
        myPID.Compute();
        pwmDutyOn = (uint) Output;
        debugV("p %0.2f i %0.2f d %0.2f output %d", myPID.GetKp(), myPID.GetKi(), myPID.GetKd(), pwmDutyOn);
    }
    if(handleSafetyFailure()) return;
    long diff = (long)(pwm_time - millis());
    if (bIsOn && (diff <= 0))
    {
        bIsOn = false;
        pwm_time = millis() + pwmDutyOff;
        debugI("OFF %d", pwm_time);
        digitalWrite(Pin_Relay_1, LOW);
    }
    diff = pwm_time - millis();
    if (!bIsOn && (diff <= 0) && pwmDutyOn > 20)
    {
        bIsOn = true;
        pwm_time = millis() + pwmDutyOn;
        debugI("ON %d", pwm_time);
        digitalWrite(Pin_Relay_1, HIGH);
    }
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
    if (lastCmd.startsWith("pidp"))
    {
        auto sp=lastCmd.substring(4);
        consKp = sp.toDouble();
        myPID.SetTunings(consKp, consKi, consKd);
    }
    if (lastCmd.startsWith("pidi"))
    {
        auto sp=lastCmd.substring(4);
        consKi = sp.toDouble();
        myPID.SetTunings(consKp, consKi, consKd);
    }
    if (lastCmd.startsWith("pidd"))
    {
        auto sp=lastCmd.substring(4);
        consKd = sp.toDouble();
        myPID.SetTunings(consKp, consKi, consKd);
    }
    if (lastCmd.startsWith("limit"))
    {
        auto sp=lastCmd.substring(5);
        auto limit = sp.toDouble();
        limit = limit > 400 ? 400 : limit;
        myPID.SetOutputLimits(0, limit);
    }
    if (lastCmd.startsWith("pwm"))
    {
        auto sp=lastCmd.substring(3);
        pwmDutyOn = sp.toInt();
        if(pwmDutyOn > pwmDutyOff){
            debugW("Duty cycle too high %d", pwmDutyOn);
            pwmDutyOn /= 2;
        } 
        debugI("On time %d", pwmDutyOn);
    }
}