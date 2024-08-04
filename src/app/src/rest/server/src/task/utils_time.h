#ifndef _UTILS_TIME_H
#define _UTILS_TIME_H


#include <cstdint>


using namespace std;


#define TIMER_SEC_TO_SEC (1)
#define TIMER_SEC_TO_MSEC (1000)
#define TIMER_MSEC_TO_MSEC (1)
#define TIMER_MSEC_TO_USEC (1000)
#define TIMER_SEC_TO_USEC (1000 * 1000)


class Timer
{
public:
    static uint64_t now();
#if 0
    static int checkTimer(bool & condition, bool expect,
                    uint64_t timeout = TIMER_TIMEOUT_DEFALUT,
                    uint64_t interval = TIMER_INTERVAL_DEFALUT);
#endif
    static void msleep(int msec);
};


#endif
