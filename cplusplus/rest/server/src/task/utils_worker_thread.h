#ifndef _UTILS_WORKER_THREAD_H
#define _UTILS_WORKER_THREAD_H


#include <mutex>
#include <condition_variable>
#include <thread>
#include "utils_worker_queue.hpp"


using namespace std;


struct Communication
{
public:
    Communication();
    virtual ~Communication();
#if 0
    int wait2();
    int wake2();
#endif
    int wait();
    int wake();
    std::mutex m_mutexQueue;
    WorkerQueue<int> m_queue;
    std::condition_variable m_conditonWorker;
    std::mutex m_mutexWorker;
    int64_t m_waiterCounter = 0;
    int64_t m_wakerCounter = 0;
};


struct WorkerThread
{
public:
    WorkerThread();
    virtual ~WorkerThread();
    template <class CallbackFn, class... CallbackArgs>
    int warpThread(std::string name,
                CallbackFn&& fn,
                CallbackArgs&&... a) {
        m_name = name;
        m_isEnable = true;
        std::thread p1 = std::thread(&WorkerThread::wrap<CallbackFn, CallbackArgs...>,
                        this,
                        std::forward<CallbackFn>(fn),
                        std::forward<CallbackArgs>( a )...);
        m_thread = std::move(p1);
        //m_thread.detach();
        return 0;
    }
    /* --comment by louting, 2023/3/15--
     * common do not for instance
     */
    template <class CallbackFn, class... CallbackArgs>
    static void wrap(WorkerThread * p_instance,
                CallbackFn&& fn,
                CallbackArgs&&... a){
        //p_instance->m_thread.detach();
        p_instance->m_stopEntry = false;
        p_instance->m_runningEntry = true;
        std::ref(fn)(std::forward<CallbackArgs>(a)...);
        p_instance->m_runningEntry = false;
        return;
    }
    int wait();
    int wake();
    int nickRun();
    int nickStop();
    bool isStop();
    int stop(bool needWaitExit=true);
    int checkRunning();
    int checkRunning(uint64_t timeout);
    int checkExited();
    int checkExited(uint64_t timeout);
public:
    std::thread m_thread;
    std::mutex m_mutex;
    bool m_isEnable = false;
    bool m_stopEntry = false;
    bool m_runningEntry = false;
    Communication m_Communication;
    std::string m_name;
};


#endif
