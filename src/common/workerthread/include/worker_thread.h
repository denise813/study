#ifndef _YS_WORKER_THREAD_H
#define _YS_WORKER_THREAD_H


#include <mutex>
#include <condition_variable>
#include <thread>
#include "memory_pool.h"


using namespace std;


namespace yuanshuo
{
namespace tools
{


struct WorkerThread
{
public:
    WorkerThread();
    virtual ~WorkerThread();
    template <class CallbackFn, class... CallbackArgs>
    int warpThread(CallbackFn&& fn, CallbackArgs&&... a) {
        std::thread p1 = std::thread(&WorkerThread::wrap<CallbackFn, CallbackArgs...>,
                        std::forward<CallbackFn>(fn),
                        std::forward<CallbackArgs>( a )...);
        m_thread = std::move(p1);
        return 0;
    }
    /* --comment by louting, 2023/3/15--
     * common do not for instance
     */
    template <class CallbackFn, class... CallbackArgs>
    static void wrap(CallbackFn&& fn, CallbackArgs&&... a){
        memorypool::MemoryPool::Instance->EnablePool();
        std::ref(fn)(std::forward<CallbackArgs>(a)...);
        memorypool::MemoryPool::Instance->DisablePool();
        return;
    }
    int wait();
    int wake();
public:
    std::thread m_thread;
    bool m_stopEntry = false;
    bool m_runningEntry = false;
    bool m_readyEntry = false;
private:
    std::condition_variable m_conditonWorker;
    std::mutex m_mutexWorker;
};


};
};


#endif
