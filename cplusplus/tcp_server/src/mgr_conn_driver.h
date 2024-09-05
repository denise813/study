#ifndef _MGR_CONN_DRIVER_H_
#define _MGR_CONN_DRIVER_H_


#include "config.h"
#include "conn_driver.h"


class TcpMgrDriver : public TcpConnDriver
{
public:
    TcpMgrDriver(int fd);
    virtual ~TcpMgrDriver();
public:
    int init(TcpConnConfig * configPtr, TcpPoller * pollerPtr);
    int exit();
public:
    void set_sched(int flags) { return; }
    int  get_sched() { return 0; }
    int handler_recv();
    int handler_send();
    int sched_handler();
protected:
    TcpConfig * m_configPtr = nullptr;
    TcpPoller * m_pollerPtr = nullptr;
};


typedef std::shared_ptr<TcpMgrDriver> TcpMgrDriverPtr;


extern int register_tcp_conn_driver(TcpConnDriverPtr & drvPtr);



#endif
