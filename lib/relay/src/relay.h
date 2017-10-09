#ifndef _RELAY_
#define _RELAY_

class Relay
{
public:
    Relay(int);
    void On();
    void Off();
    bool IsOn();
    void Toggle();
private:
    int RelayPin;
    bool isOn;
};

#endif /* end of include guard:  */
