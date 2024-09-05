#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/epoll.h>


#include "tcp_conn.h"


int TcpConnMgr::init()
{
    int rc = 0;
    int ep_fd = 0;

    ep_fd = epoll_create(4096);
    if (ep_fd < 0) {
        rc = -errno;
        return rc;
    }
    m_ep_fd = ep_fd;

    return 0;
}

int TcpConnMgr::exit()
{
    close(m_ep_fd);
    m_ep_fd = -1;
    return 0;
}

int TcpConnMgr::loop()
{
    event_loop();
    return 0;
}

int TcpConnMgr::exec_scheduled(void)
{
    TcpConnDriverPtr event_drvPtr = nullptr;
    for (auto itor = m_sched_map.begin(); itor != m_sched_map.end();) {
        event_drvPtr = (*itor);
        event_drvPtr->sched_handler();
        itor = m_sched_map.erase(itor);
    }

    return 0;
}

int TcpConnMgr::event_loop()
{
    int rc = 0;
    int nevent = 0;
    int i = 0;
    struct epoll_event events[1024];
    TcpConnDriver * event_drvPtr = nullptr;
    
    while(1) {
        exec_scheduled();
        nevent = epoll_wait(m_ep_fd, events, 1024, 0);
        if (nevent < 0) {
            rc = -errno;
            if (rc != -EINTR) {
                return rc;
            }
            continue;
        }
        for (i = 0; i < nevent; i++) {
            event_drvPtr = (TcpConnDriver *) events[i].data.ptr;
            if (events[i].events == EPOLLIN) {
                event_drvPtr->handler_recv();
            } else if (events[i].events == EPOLLOUT) {
                event_drvPtr->handler_send();
            } else {
                //
            }
        }
    }

    return 0;
}

int TcpConnMgr::add_event(int events, TcpConnDriverPtr &drvPtr)
{
    struct epoll_event ev;
    int rc = -1;
    int fd = -1;

    TcpConnDriverPtr event_drvPtr = drvPtr;
    fd = event_drvPtr->get_fd();

    memset(&ev, 0, sizeof(ev));
    ev.events = events;
    ev.data.ptr = event_drvPtr.get();
    rc = epoll_ctl(m_ep_fd, EPOLL_CTL_ADD, fd, &ev);
    if (rc < 0) {
        rc = -errno;
        goto l_out;
   }
    m_conn_map.insert(std::pair<int,TcpConnDriverPtr>(fd, event_drvPtr));
    rc = 0;

l_out:
    return rc;
}

TcpConnDriverPtr TcpConnMgr::lookup_event(int fd)
{
    TcpConnDriverPtr event_drvPtr;
    auto itor = m_conn_map.find(fd);
    if (itor == m_conn_map.end()) {
        goto l_out;
    }
    event_drvPtr = itor->second;

l_out:
    return event_drvPtr;
}

void TcpConnMgr::del_event(int fd)
{
    int rc = 0;
    TcpConnDriverPtr event_drvPtr;

    event_drvPtr = lookup_event(fd);
    if (!event_drvPtr) {
        goto l_out;
    }

    rc = epoll_ctl(m_ep_fd, EPOLL_CTL_DEL, fd, NULL);
    if (rc < 0){
        rc = -errno;
        goto l_out;
    }

     m_conn_map.erase(fd);
l_out:
    return;
}

int TcpConnMgr::modify_event(int fd, int events)
{
    int rc = 0;
    TcpConnDriverPtr event_drvPtr;
    struct epoll_event ev;

    event_drvPtr = lookup_event(fd);
    if (!event_drvPtr) {
        rc = -ENOENT;
        return rc;
    }

    memset(&ev, 0, sizeof(ev));
    ev.events = events;
    ev.data.ptr = event_drvPtr.get();

    return epoll_ctl(m_ep_fd, EPOLL_CTL_MOD, fd, &ev);
}

void TcpConnMgr::add_sched_event(TcpConnDriverPtr &drvPtr)
{
    drvPtr->set_sched(1);
    m_sched_map.push_back(drvPtr);
}

void TcpConnMgr::remove_sched_event(TcpConnDriverPtr &drvPtr)
{
    TcpConnDriverPtr event_drvPtr = drvPtr;
    event_drvPtr->set_sched(0);
    m_conn_map.erase(event_drvPtr->get_fd());
}


