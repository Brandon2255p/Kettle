#include "alarm.h"
#include <Arduino.h>

Alarm::Alarm()
{
    previousMillis = millis();
}

void Alarm::SetupAlarm(int minutes, int seconds, std::function<void(void)> callback)
{
    callbackFunction = callback;
    int total_time = (minutes * 60 + seconds) * 1000;
    bool belowMinInterval = total_time < minInterval;
    if(belowMinInterval)
    {
        interval = minInterval;
    }
    else
    {
        interval = total_time;
    }
}

void Alarm::HandleAlarm()
{
    if(alarmRunning)
    {
        unsigned long currentMillis = millis();
        if (currentMillis - previousMillis >= interval) {
            previousMillis = currentMillis;
            callbackFunction();
        }
    }
}

void Alarm::ResetAlarm()
{
    Serial.println("ALARM: Resetting...");
    previousMillis = millis();
    alarmRunning = true;
}
