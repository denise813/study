
#include "client_conn_driver.h"


TcpClientDriver::TcpClientDriver(int fd)
{
    set_fd(fd);
    set_name("client");
}

TcpClientDriver::~TcpClientDriver()
{
}

int TcpClientDriver::init(TcpConnConfig * configPtr, TcpPoller * pollerPtr)
{
    m_configPtr = configPtr;
    m_pollerPtr = pollerPtr;
    return 0;
}

int TcpClientDriver::exit()
{
    return 0;
}

int TcpClientDriver::handler_recv()
{
    // create task
    return 0;
}

int TcpClientDriver::handler_send()
{
    return 0;
}

int TcpClientDriver::sched_handler()
{
    return 0;
}

