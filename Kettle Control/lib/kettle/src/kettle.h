#ifndef  __KETTLE__
#define __KETTLE__
#include <relay.h>
#include <alarm.h>
#include <KettleTempControl.h>

class Kettle
{
public:
    Kettle();
    void StartBoiling();
    void StopBoiling();
    void Handle();
private:
    bool isBoiling;
    Relay KettleRelay;
    Alarm NoOverBoilAlarm;
    KettleTempControl kettleTempControl;
    void NoOverBoil_AlarmCallback();
};

#endif /* end of include guard: __KETTLE__ */
