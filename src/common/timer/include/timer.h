#ifndef _YS_TIMER_H
#define _YS_TIMER_H


#include <cstdint>


using namespace std;


namespace yuanshuo
{
namespace tools
{


#define TIMER_SEC_TO_SEC (1)
#define TIMER_SEC_TO_MSEC (1000)
#define TIMER_MSEC_TO_MSEC (1)
#define TIMER_MSEC_TO_USEC (1000)
#define TIMER_SEC_TO_USEC (1000 * 1000)

#define TIMER_TIMEOUT_DEFALUT (3 * TIMER_SEC_TO_MSEC)
#define TIMER_INTERVAL_DEFALUT (100 * TIMER_MSEC_TO_MSEC)
class Timer
{
public:
    static uint64_t now();
    static int checkTimer(bool & condition, bool expect,
                    uint64_t timeout = TIMER_TIMEOUT_DEFALUT,
                    uint64_t interval = TIMER_INTERVAL_DEFALUT);
    static void msleep(int msec);
};


};
};


#endif
