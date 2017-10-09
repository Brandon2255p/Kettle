#ifndef __ALARM__
#define __ALARM__

#include <functional>
typedef void(*alarmcallback)(void);

class Alarm
{
public:
    Alarm();
    void SetupAlarm(int minutes, int seconds, std::function<void(void)> callback);
    void HandleAlarm();
    void ResetAlarm();
private:
    long interval;
    long previousMillis;
    const long minInterval = 1000;
    std::function<void(void)> callbackFunction;
    bool alarmRunning;
};

#endif /* end of include guard: __ALARM__ */
