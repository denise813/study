#ifndef _TCP_CONN_H
#define _TCP_CONN_H


#include <memory>
#include <map>
#include <list>


#include "tcp_conn_driver.h"


using namespace std;


class TcpConnMgr : public TcpDriverMgr
{
public:
    TcpConnMgr() = default;
    virtual ~TcpConnMgr() = default;
public:
    int init();
    int exit();
    int loop();
public:
    int add_event(int events, TcpConnDriverPtr &drvPtr);
    void del_event(int fd);
    int modify_event(int fd, int events);
private:
    int exec_scheduled(void);
    int event_loop();
    TcpConnDriverPtr lookup_event(int fd);
    void add_sched_event(TcpConnDriverPtr &drvPtr);
    void remove_sched_event(TcpConnDriverPtr &drvPtr);
    
private:
    int m_ep_fd = -1;
    std::map<int, TcpConnDriverPtr> m_conn_map;
    std::list<TcpConnDriverPtr> m_sched_map;
};


typedef std::shared_ptr<TcpConnMgr> TcpConnMgrPtr;


#endif
