#include "worker_threadpool.h"


using namespace std;


WorkerThreadPool::WorkerThreadPool(int threadnum, std::string name)
{
    m_name = name;
    m_threadnum = threadnum;
}

WorkerThreadPool::~WorkerThreadPool()
{
}

