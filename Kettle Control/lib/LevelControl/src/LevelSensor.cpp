#include "LevelSensor.h"
#include <Arduino.h>

void LevelSensor::Handle()
{
    int reading = digitalRead(sensorPin);
    if(reading == HIGH)
        callbackFunction();
}

void LevelSensor::Setup(int pin, std::function<void(void)> callback)
{
    callbackFunction = callback;
    sensorPin = pin;
    pinMode(sensorPin, INPUT_PULLUP);
}
