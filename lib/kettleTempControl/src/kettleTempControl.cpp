#include "kettleTempControl.h"

void KettleTempControl::Handle()
{
    int currentTemp = ReadTemp();
    bool overLimit = currentTemp > tempLimit;
    if(overLimit)
    {
        callbackFunction();
    }
}

void KettleTempControl::Setup(int tempLimit, std::function<void(void)> callback)
{
    this->tempLimit = tempLimit;
}

int KettleTempControl::ReadTemp()
{
    return 100;
}
