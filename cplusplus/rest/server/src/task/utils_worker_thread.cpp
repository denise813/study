#include<iostream>
#include "utils_time.h"
#include "utils_worker_thread.h"


using namespace std;


#define WORKER_THREAD_TIMER_TIMEOUT_DEFALUT (10 * TIMER_SEC_TO_MSEC)
#define WORKER_THREAD_TIMER_INTERVAL_DEFALUT (100 * TIMER_MSEC_TO_MSEC)


Communication::Communication()
{
}

Communication::~Communication()
{
}

int Communication::wait()
{
    std::unique_lock <std::mutex> waitLock(m_mutexWorker);
    int64_t oldWakerCounter = m_wakerCounter;
    m_waiterCounter = m_waiterCounter + 1;
    //std::cout << "wait start m_waiterCounter ="  << m_waiterCounter << " m_wakerCounter = " <<m_wakerCounter << std::endl;
    if (m_waiterCounter > oldWakerCounter) {
        m_conditonWorker.wait(waitLock);
    }
    m_wakerCounter = m_waiterCounter;
    //std::cout << "wait end m_waiterCounter ="  << m_waiterCounter << " m_wakerCounter = " <<m_wakerCounter << std::endl;
    return 0;
}

int Communication::wake()
{
    std::unique_lock <std::mutex> waitLock(m_mutexWorker);
    m_wakerCounter = m_wakerCounter + 1;
    //std::cout << "wake start m_waiterCounter ="  << m_waiterCounter << " m_wakerCounter = " <<m_wakerCounter << std::endl;
    m_conditonWorker.notify_one();
    //std::cout << "wake end start m_waiterCounter ="  << m_waiterCounter << " m_wakerCounter = " <<m_wakerCounter << std::endl;
    return 0;
}

#if 0
int Communication::wait2()
{
    std::unique_lock <std::mutex> waitLock(m_mutexWorker);
    m_conditonWorker.wait(waitLock);
    return 0;
}

int Communication::wake2()
{
    std::unique_lock <std::mutex> waitLock(m_mutexWorker);
    m_conditonWorker.notify_one();
    return 0;
}
#endif

WorkerThread::WorkerThread()
{
}

WorkerThread::~WorkerThread()
{
    if (m_isEnable == true) {
        stop();
#if 1
        if (m_thread.joinable()) {
            m_thread.join();
        }
#endif
    }
    m_isEnable = false;
}

int WorkerThread::wait()
{
    m_Communication.wait();
    return 0;
}

int WorkerThread::nickRun()
{
    m_stopEntry = false;
    m_runningEntry = true;
    wait();
    return 0;
}
int WorkerThread::nickStop()
{
    if (m_stopEntry) {
        return 0;
    }
    wake();
    m_runningEntry = false;
    m_stopEntry = true;
    return 0;
}


int WorkerThread::checkRunning(uint64_t timeout)
{
    int rc = 0;
    uint64_t startTimer = Timer::now();
    uint64_t currentTimer = startTimer;
    uint64_t interval = WORKER_THREAD_TIMER_INTERVAL_DEFALUT;
    while (1) {
        //currentTimer = Timer::now();
         if (m_runningEntry == true) {
            break;
        }
#if 1
        if (currentTimer -startTimer > timeout) {
            rc = -ETIMEDOUT;
            break;
        }
 #endif
        Timer::msleep(interval);
    }
    return rc;
}

int WorkerThread::checkRunning()
{
    uint64_t timeout = WORKER_THREAD_TIMER_TIMEOUT_DEFALUT;
    return checkRunning(timeout);
}

int WorkerThread::checkExited(uint64_t timeout)
{
    int rc = 0;
    //uint64_t startTimer = Timer::now();
    //uint64_t currentTimer = startTimer;
    uint64_t interval = WORKER_THREAD_TIMER_INTERVAL_DEFALUT;
    while (1) {
        //currentTimer = Timer::now();
         if (m_runningEntry == false) {
            break;
        }
#if 0
        if (currentTimer -startTimer > timeout) {
            rc = -ETIMEDOUT;
            break;
        }
#endif
        if (m_stopEntry == true) {
            m_Communication.wake();
        }
        Timer::msleep(interval);
    }
    return rc;
}

int WorkerThread::checkExited()
{
    uint64_t timeout = WORKER_THREAD_TIMER_TIMEOUT_DEFALUT;
    return checkExited(timeout);
}

int WorkerThread::wake()
{
    m_Communication.wake();
    return 0;
}

bool WorkerThread::isStop()
{
    return m_stopEntry == true;
}

int WorkerThread::stop(bool needWaitExit)
{
    m_stopEntry = true;
    m_Communication.wake();
    if (needWaitExit == true) {
        checkExited();
    }
    return 0;
}
