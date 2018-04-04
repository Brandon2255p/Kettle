#ifndef __LEVEL_CONTROL__
#define __LEVEL_CONTROL__
#include <functional>

class LevelSensor
{
public:
    void Setup(int pin, std::function<void(void)> callback);
    void Handle();

private:
    std::function<void(void)> callbackFunction;
    int sensorPin;
};

#endif /* end of include guard: __LEVEL_CONTROL__ */
