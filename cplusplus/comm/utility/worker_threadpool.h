#ifndef _WORKER_THREADPOOL_H
#define _WORKER_THREADPOOL_H


#include <vector>
#include "ys_string_format.hpp"
#include "ys_worker_thread.h"


using namespace std;


struct WorkerThreadPool
{
public:
    WorkerThreadPool(int threadnum, std::string name);
    virtual ~WorkerThreadPool();
    template <class CallbackFn, class... CallbackArgs>
    int warpThreadPool(
                CallbackFn&& fn,
                CallbackArgs&&... a) {
        int rc = 0;
        int hasfailed = false;
        for (int i = 0; i < m_threadnum; i++) {
            std::shared_ptr<WorkerThread> entryPtr = std::make_shared<WorkerThread>();
            m_threads.push_back(entryPtr);
        }

        for (int i = 0; i < m_threadnum; i++) {
            std::string threadname = string_format("%s-%d", m_name.c_str(), i);
            m_threads[i]->warpThread(threadname, std::forward<CallbackFn>(fn),
                        std::forward<CallbackArgs>( a )...);
        }

        for (int i = 0; i < m_threadnum; i++) {
            rc = m_threads[i]->checkRunning();
            if (rc < 0) {
                hasfailed = true;
                break;
            }
        }

        if (hasfailed == true) {
            for (int i = 0; i < m_threadnum; i++) {
                m_threads[i]->stop();
            }
        }

        return 0;
    }

private:
    int m_threadnum = 1;
    std::vector< std::shared_ptr<WorkerThread> > m_threads;
    std::string m_name;
};


#endif
