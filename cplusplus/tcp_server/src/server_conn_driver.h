#ifndef _SERVER_CONN_DRIVER_H_
#define _SERVER_CONN_DRIVER_H_


#include "config.h"
#include "epoller.h"


class TcpServerDriver : public TcpConnDriver
{
public:
    TcpServerDriver(int fd);
    ~TcpServerDriver();
public:
    int init(TcpConnConfig * configPtr, TcpPoller * pollerPtr);
    int exit();
public:
    virtual void set_sched(int flag) {}
    virtual int get_sched() { return 0; }
    int handler_recv();
    int handler_send();
    int sched_handler();
private:
    TcpConfig * m_configPtr = nullptr;
    TcpEPollerMgr * m_pollerPtr = nullptr;
};


typedef std::shared_ptr<TcpServerDriver> TcpServerDriverPtr;



#endif
