#ifndef _WORKER_Q_H
#define _WORKER_Q_H


#include <memory>
#include <mutex>
#include <list>


using namespace std;


template <class T>
struct WorkerQueue
{
public:
    typedef typename std::list<T>::iterator Iterator;
    WorkerQueue() {
    }
    ~WorkerQueue() {
        std::unique_lock <std::mutex> submitLock(m_mutex);
        m_list.resize(0);
    }
    void addEntry(T entry) {
        std::unique_lock <std::mutex> submitLock(m_mutex);
        m_list.push_back(entry);
    }

    bool isEmpty(Iterator itor)
    {
         std::unique_lock <std::mutex> submitLock(m_mutex);
         if (itor == m_list.end()) {
            return true;
         }
         return false;
    }
    
    Iterator getEntryItor() {
        std::unique_lock <std::mutex> submitLock(m_mutex);
        Iterator itor = m_list.end();
        if (!m_list.empty()) {
            Iterator itor = m_list.begin();
            m_list.pop_front();
            m_list.erase(itor);
        }
        return itor;
    }

    std::mutex m_mutex;
    std::list<T> m_list;
};


#endif
