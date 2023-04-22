#include "ys_worker_thread.h"


using namespace std;
using namespace yuanshuo;
using namespace yuanshuo::tools;


WorkerThread::WorkerThread()
{
}

WorkerThread::~WorkerThread()
{
    m_thread.join();
}

int WorkerThread::wait()
{
    std::unique_lock <std::mutex> waitLock(m_mutexWorker);
    m_conditonWorker.wait(waitLock);
    return 0;
}

int WorkerThread::wake()
{
    m_conditonWorker.notify_one();
    return 0;
}
