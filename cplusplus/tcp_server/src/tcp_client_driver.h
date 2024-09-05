#ifndef _TCP_CLIENT_DRIVER_H_
#define _TCP_CLIENT_DRIVER_H_


class TcpClientDriver : public TcpConnDriver
{
public:
    TcpConnDriver() = default;
    ~TcpConnDriver() = default;
public:
    int init(TcpConnConfig * configPtr, TcpDriverMgr * connMgrPtr) { return 0; }
    int exit() { return 0; }
public:
    int handler_recv() { return 0; }
    int handler_send() { return 0; }
    int sched_handler() { return 0; }
};


typedef std::shared_ptr<TcpConnDriver> TcpConnDriverPtr;


extern int register_tcp_conn_driver(TcpConnDriverPtr & drvPtr);



#endif
