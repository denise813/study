#ifndef _CLIENT_CONN_DRIVER_H_
#define _CLIENT_CONN_DRIVER_H_


#include <memory>

#include "conn_driver.h"


class TcpClientDriver : public TcpConnDriver
{
public:
    TcpClientDriver(int fd);
    ~TcpClientDriver();
public:
    int init(TcpConnConfig * configPtr, TcpPoller * pollerPtr);
    int exit();
public:
    void set_sched(int flags) { return; }
    int  get_sched() { return 0; }
    int handler_recv();
    int handler_send();
    int sched_handler();
private:
    TcpConnConfig * m_configPtr=nullptr;
    TcpPoller * m_pollerPtr =nullptr;
};


typedef std::shared_ptr<TcpClientDriver> TcpClientDriverPtr;


extern int register_tcp_conn_driver(TcpConnDriverPtr & drvPtr);



#endif
