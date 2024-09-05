#include "tcp_mgr_driver.h"


TcpMgrDriver::TcpMgrDriver()
{
}

TcpMgrDriver::~TcpMgrDriver()
{
}

int TcpMgrDriver::init(TcpConnConfig * configPtr, TcpDriverMgr * connMgrPtr)
{
    TcpConfig * tcp_configPtr = dynamic_cast<TcpConfig*>(configPtr);
    m_configPtr = tcp_configPtr;
    m_connMgrPtr = connMgrPtr;
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

