#ifndef _TCP_MGR_DRIVER_H_
#define _TCP_MGR_DRIVER_H_


#include "tcp_config.h"
#include "tcp_conn_driver.h"


class TcpMgrDriver : public TcpConnDriver
{
public:
    TcpMgrDriver();
    virtual ~TcpMgrDriver();
public:
    int init(TcpConnConfig * configPtr, TcpDriverMgr * connMgrPtr);
    int exit();
public:
    void set_sched(int flags) { return; }
    int  get_sched() { return 0; }
    int handler_recv();
    int handler_send();
    int sched_handler();
protected:
    TcpConfig * m_configPtr = nullptr;
    TcpDriverMgr * m_connMgrPtr = nullptr;
};


typedef std::shared_ptr<TcpMgrDriver> TcpMgrDriverPtr;


extern int register_tcp_conn_driver(TcpConnDriverPtr & drvPtr);



#endif
