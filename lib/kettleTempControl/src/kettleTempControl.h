#ifndef __KETTLE_TEMP_CONTROL__
#define __KETTLE_TEMP_CONTROL__
#include <functional>

class KettleTempControl
{
public:
    void Handle();
    void Setup(int tempLimit, std::function<void(void)> callback);
private:
    std::function<void(void)> callbackFunction;
    int tempLimit;
    int ReadTemp();
};


#endif /* end of include guard: __KETTLE_TEMP_CONTROL__ */
