#ifndef _TCP_CONN_DRIVER_H_
#define _TCP_CONN_DRIVER_H_


#include "tcp_conn_config.h"


class TcpDriverMgr
{
public:
    TcpDriverMgr() = default;
    virtual ~TcpDriverMgr() = default;
};

typedef std::shared_ptr<TcpDriverMgr> TcpDriverMgrPtr;


class TcpConnDriver
{
public:
    TcpConnDriver() = default;
    virtual ~TcpConnDriver() = default;
public:
    virtual int init(TcpConnConfig * configPtr, TcpDriverMgr * connMgrPtr) = 0;
    virtual int exit() = 0;
public:
    void set_name(std::string  name) { m_name = name; }
    std::string get_name() { return m_name; }
    int get_fd() { return m_fd; }
    void set_fd(int fd) { m_fd = fd; }
    virtual void set_sched(int flag) = 0;
    virtual int get_sched() = 0;
    virtual int handler_recv() = 0;
    virtual int handler_send() = 0;
    virtual int sched_handler() = 0;
private:
    int m_fd = -1;
    std::string m_name;
};


typedef std::shared_ptr<TcpConnDriver> TcpConnDriverPtr;


extern int register_conn_driver(TcpConnDriverPtr & drvPtr);


#endif
