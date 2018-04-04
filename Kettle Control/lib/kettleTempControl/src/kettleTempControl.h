#ifndef __KETTLE_TEMP_CONTROL__
#define __KETTLE_TEMP_CONTROL__
#include <functional>
#include <OneWire.h>
#include <DallasTemperature.h>

class KettleTempControl
{
public:
    KettleTempControl();
    void Handle();
    void Setup(int tempLimit, std::function<void(void)> callback);
private:
    std::function<void(void)> callbackFunction;
    int tempLimit;
    int ReadTemp();
    OneWire sensorBus;
    DallasTemperature sensors;
    DeviceAddress insideThermometer;
};


#endif /* end of include guard: __KETTLE_TEMP_CONTROL__ */
