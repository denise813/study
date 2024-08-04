#include "utils_time.h"
#include "utils_worker_threadpool.h"


WorkerThreadPool::WorkerThreadPool(int threadnum, std::string name)
{
    m_name = name;
    m_threadnum = threadnum;
}

WorkerThreadPool::~WorkerThreadPool()
{
}

