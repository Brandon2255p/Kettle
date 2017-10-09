#include "relay.h"
#include <Arduino.h>

Relay::Relay(int i)
{
    RelayPin = i;
    pinMode(RelayPin, OUTPUT);
    digitalWrite(RelayPin, LOW);
}

void Relay::On()
{
    digitalWrite(RelayPin, HIGH);
    isOn = true;
}

void Relay::Off()
{
    digitalWrite(RelayPin, LOW);
    isOn = false;
}

bool Relay::IsOn()
{
    return isOn;
}

void Relay::Toggle()
{
    if(isOn)
        Off();
    else
        On();
}
