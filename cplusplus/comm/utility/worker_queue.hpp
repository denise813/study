#ifndef _WORKER_QUEUE_HPP_
#define _WORKER_QUEUE_HPP_

#include <memory>
#include <mutex>
#include <condition_variable>
#include <list>

using namespace std;

namespace yuanshuo
{
namespace tools
{


template <class T>
struct WorkerQueue
{
private:
     std::mutex m_mutex;
    std::list<T> m_list;
public:
    typedef typename std::list<T>::iterator Iterator;
    WorkerQueue() {
    }
    ~WorkerQueue() {
        std::unique_lock <std::mutex> submitLock(m_mutex);
        m_list.resize(0);
    }

    int getSize() {
        return m_list.size();
    }
    void addEntry(T entry) {
        std::unique_lock <std::mutex> submitLock(m_mutex);
        m_list.push_back(entry);
    }

    void addHeadEntry(T entry) {
        std::unique_lock <std::mutex> submitLock(m_mutex);
        m_list.push_front(entry);
    }

#if 1
    bool isEmpty()
    {
         std::unique_lock <std::mutex> submitLock(m_mutex);
         auto itor = m_list.begin();
         if (itor == m_list.end()) {
            return true;
         }
         return false;
    }
#endif

#if 1
    int getEntry(T &entry) {
        std::unique_lock <std::mutex> submitLock(m_mutex);
        auto itor = m_list.begin();
        if (itor == m_list.end()) {
            return -ENOENT;
        }
        entry = (*itor);
        m_list.pop_front();
        return 0;
    }
#endif

#if 0
    Iterator getEntryItor() {
        std::unique_lock <std::mutex> submitLock(m_mutex);
        Iterator itor = m_list.end();
        if (!m_list.empty()) {
            itor = m_list.begin();
            m_list.pop_front();
        }
        return itor;
    }
#endif

    void addEntry(T* entry) {
        std::unique_lock <std::mutex> submitLock(m_mutex);
        m_list.push_back(entry);
    }

    int getEntry(T** entry) {
        std::unique_lock <std::mutex> submitLock(m_mutex);
        auto itor = m_list.begin();
        if (itor == m_list.end()) {
            return -ENOENT;
        }
        *entry = (*itor);
        m_list.pop_front();
        return 0;
    }

    void addHeadEntry(T* entry) {
        std::unique_lock <std::mutex> submitLock(m_mutex);
        m_list.push_front(entry);
    }

};


};
};



#endif
