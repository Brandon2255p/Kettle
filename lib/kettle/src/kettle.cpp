#include "kettle.h"
#include <Arduino.h>

Kettle::Kettle():
    KettleRelay(12)
{
    auto callback = std::bind(&Kettle::NoOverBoil_AlarmCallback, this);
    NoOverBoilAlarm.SetupAlarm(1, 0, callback);
}

void Kettle::StartBoiling()
{
    if(!isBoiling){
        NoOverBoilAlarm.ResetAlarm();
        KettleRelay.On();
        isBoiling = true;
    }
}

void Kettle::StopBoiling()
{
    isBoiling = false;
    KettleRelay.Off();
}

void Kettle::NoOverBoil_AlarmCallback(){
    KettleRelay.Off();
    isBoiling = false;
    Serial.println("ALARM: NoOverBoil");
}

void Kettle::Handle()
{
    if(!isBoiling)//ensure off is off
    {
        KettleRelay.Off();
    }
    else
    {
        NoOverBoilAlarm.HandleAlarm();
    }
}
