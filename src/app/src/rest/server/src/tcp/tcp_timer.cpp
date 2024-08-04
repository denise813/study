#include "tcp_timer.h"


TcpTimerConext::TcpTimerConext(int fd, int timeout, int expire)
{
    m_fd = fd;
    m_timeout = timeout;
    m_expire = expire;
}


TcpAlrmTimer::TcpAlrmTimer(int epfd, int notifyfd)
{
    m_epfd = epfd;
    m_notifyfd = notifyfd;
}

TcpAlrmTimer::~TcpAlrmTimer()
{
}

int TcpAlrmTimer::init()
{
    return 0;
}

int TcpAlrmTimer::exit()
{
    return 0;
}

TcpAlrmTimer::createEvent(int fd, int timeout)
{
    time_t cur = time(NULL);
    int expire = cur + interval;
    TimerConextPtr timePtr = std::make_shared<TimerConext>(fd, timeout , expire);
    m_timers.push_back(timePtr);
}

int TcpAlrmTimer::destroyEvent(int fd)
{
    for(auto itor : m_timers) {
        if (fd != itor->m_fd) {
            continue;
        }
        itor = earse(itor);
        break;
    }
    return 0;
}

int TcpAlrmTimer::updateEvent(int fd)
{
    for(auto itor : m_timers) {
        if (fd != itor->m_fd) {
            continue;
        }
        time_t cur = time(NULL);
        itor->m_expire = cur + itor->m_timeout;
        break;
    }
    return 0;
}

int TcpAlrmTimer::setKeepAlive(int fd)
{
    for(auto itor : m_timers) {
        if (fd != itor->m_fd) {
            continue;
        }
        time_t cur = time(NULL);
        itor->m_expire = -1;
        break;
    }
    return 0;
    return 0;
}

int TcpAlrmTimer::timeoutEvent()
{
    SocketAPI::modify_epoll_event(m_epfd, m_notifyfd, EPOLLIN);
    return 0;
}

int TcpAlrmTimer::entry()
{
    for(auto itor : m_timers) {
        time_t cur = time(NULL);
        if (cur > itor->expire && itor->expire != -1) {
            timeoutEvent();
        }
    }
    return 0;
}

