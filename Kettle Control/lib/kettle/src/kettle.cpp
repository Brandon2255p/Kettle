#include "kettle.h"
#include <Arduino.h>
#include <ServerSentEvent.h>
Kettle::Kettle():
    KettleRelay(12),
    signalBoilComplete(false)
{
    auto callback = std::bind(&Kettle::NoOverBoil_AlarmCallback, this);
    NoOverBoilAlarm.SetupAlarm(3, 30, callback);
    kettleTempControl.Setup(75, callback);
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

bool Kettle::SignalBoilComplete()
{
    if(signalBoilComplete)
    {
        signalBoilComplete = false;
        return true;
    }
    return false;
}

void Kettle::NoOverBoil_AlarmCallback(){
    if(isBoiling){
        sseSerial.println("ALARM: NoOverBoil");
        signalBoilComplete = true;
    }
    KettleRelay.Off();
    isBoiling = false;
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
    kettleTempControl.Handle();
}
