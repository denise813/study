#include "mgr_conn_driver.h"


TcpMgrDriver::TcpMgrDriver(int fd)
{
    set_fd(fd);
    set_name("mgr");
}

TcpMgrDriver::~TcpMgrDriver()
{
}

int TcpMgrDriver::init(TcpConnConfig * configPtr, TcpPoller * pollerPtr)
{
    TcpConfig * tcp_configPtr = dynamic_cast<TcpConfig*>(configPtr);
    m_configPtr = tcp_configPtr;
    m_pollerPtr = pollerPtr;
    return 0;
}

int TcpMgrDriver::exit()
{
    return 0;
}

int TcpMgrDriver::handler_recv()
{
    return 0;
}

int TcpMgrDriver::handler_send()
{
    return 0;
}

int TcpMgrDriver::sched_handler()
{
    return 0;
}

