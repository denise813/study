#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include "timer_tools.h"


uint64_t Timer::now()
{
    struct timeval tv;
    struct timezone tz;
    gettimeofday(&tv,&tz);
    uint64_t timerNow = tv.tv_sec*1000 + tv.tv_usec/1000;
    return timerNow;
}

#if 0
int Timer::checkTimer(bool & condition, bool expect,
                uint64_t ms_timeout,
                uint64_t ms_interval)
{
    int rc = 0;
    uint64_t interval = ms_interval * TIMER_MSEC_TO_USEC;
    uint64_t timeout = ms_timeout * TIMER_MSEC_TO_USEC;
    uint64_t startTimer = Timer::now();
    uint64_t currentTimer = startTimer;
    while (1) {
        currentTimer = Timer::now();
        if (condition == expect) {
            rc = currentTimer -startTimer;
            break;
        }
        if (currentTimer -startTimer > timeout) {
            // log 
            rc = -ETIMEDOUT;
            break;
        }
        usleep(interval);
    }

    return rc;
}
#endif

void Timer::msleep(int msec)
{
    int interval = msec * TIMER_MSEC_TO_USEC;
    usleep(interval);
    return;
}

