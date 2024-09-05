#ifndef _CONN_DRIVER_H_
#define _CONN_DRIVER_H_


#include <string>
#include <memory>

#include "conn_config.h"


using namespace std;


class TcpPoller;
class TcpConnDriver
{
public:
    TcpConnDriver() = default;
    virtual ~TcpConnDriver() = default;
public:
    virtual int init(TcpConnConfig * configPtr, TcpPoller * connMgrPtr) = 0;
    virtual int exit() = 0;
public:
    std::string get_name() { return m_name; }
    int get_fd() { return m_fd; }
    virtual void set_sched(int flag) = 0;
    virtual int get_sched() = 0;
    virtual int handler_recv() = 0;
    virtual int handler_send() = 0;
    virtual int sched_handler() = 0;
protected:
     void set_name(std::string  name) { m_name = name; }
     void set_fd(int fd) { m_fd = fd; }
private:
    int m_fd = -1;
    std::string m_name;
};


typedef std::shared_ptr<TcpConnDriver> TcpConnDriverPtr;


class TcpPoller
{
public:
    TcpPoller() = default;
    virtual ~TcpPoller() = default;
public:
    virtual int add_event(int events, TcpConnDriverPtr &drvPtr) = 0;
    virtual void del_event(int fd) = 0;
    virtual int modify_event(int fd, int events) = 0;
};

typedef std::shared_ptr<TcpPoller> TcpPollerPtr;



//extern int register_conn_driver(TcpConnDriverPtr & drvPtr);


#endif
