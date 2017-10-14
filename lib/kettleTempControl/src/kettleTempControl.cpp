#include "kettleTempControl.h"
#include <ServerSentEvent.h>

KettleTempControl::KettleTempControl():
    sensorBus(14),
    sensors(&sensorBus)
{
    sensors.begin();
}

void KettleTempControl::Handle()
{
    int currentTemp = ReadTemp();
    bool overLimit = currentTemp > tempLimit;
    if(overLimit)
    {
        sseSerial.print("OVER TEMP");
        callbackFunction();
    }
}

void KettleTempControl::Setup(int tempLimit, std::function<void(void)> callback)
{
    sensors.begin();
    if (!sensors.getAddress(insideThermometer, 0)) Serial.println("Unable to find address for Device 0");
    this->tempLimit = tempLimit;
    callbackFunction = callback;
}

int KettleTempControl::ReadTemp()
{
    sensors.requestTemperatures();
    delay(300);
    float tempC = sensors.getTempC(insideThermometer);
    bool alarmed = sensors.alarmSearch(insideThermometer);
    sseSerial.sendEvent(String(tempC), "temperature");
    return tempC;
}
