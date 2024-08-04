#ifndef TCP_TIMER
#define TCP_TIMER


#include "task/utils_worker_queue.hpp"


class TcpTimerConext
{
public:
    TcpTimerConext(int fd, int timeout, int expire) {
        m_expire = expire;
        
    }
    ~TcpTimerConext() = default;
public:
    int m_fd;
    int m_timeout;
    int m_expire;
};


typedef std::shared_ptr<TcpTimerConext> TcpTimerConextPtr;


class TcpAlrmTimer
{
public:
    TcpAlrmTimer(int epfd, int notifyfd);
    ~TcpAlrmTimer() = default;
public:
    int init();
    int exit();
public:
    int createEvent(int fd, int timeout);
    int destroyEvent(int fd);
    int setKeepAlive(int fd);
    int updateEvent(int fd);
private:
    int timeoutEvent(int fd);
private:
    int entry();
private:
    int m_epfd;
    int m_notifyfd = -1;
    std::list<TcpTimerConextPtr> m_timers;
};


#endif
