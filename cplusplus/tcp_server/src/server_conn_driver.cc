#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/epoll.h>
#include <sys/socket.h>


#include <memory>

#include "client_conn_driver.h"
#include "server_conn_driver.h"


static int set_keepalive(int fd)
{
    int opt = 1;

    opt = 1;
    setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &opt, sizeof(opt));
    opt = 1800;
    setsockopt(fd, IPPROTO_TCP, TCP_KEEPIDLE, &opt, sizeof(opt));
    opt = 6;
    setsockopt(fd, IPPROTO_TCP, TCP_KEEPCNT, &opt, sizeof(opt));
    opt = 300;
    setsockopt(fd, IPPROTO_TCP, TCP_KEEPINTVL, &opt, sizeof(opt));

    return 0;
}

static int set_nodelay(int fd)
{
	int rc = 0;
    int opt = 1;

    setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(opt));

    return rc;
}

static int set_non_blocking(int fd)
{
    int rc = 0;

    rc = fcntl(fd, F_GETFL);
    if (rc < 0) {
        return rc;
    }
    
    rc = fcntl(fd, F_SETFL, rc | O_NONBLOCK);
    return rc;
}


TcpServerDriver::TcpServerDriver(int fd)
{
    set_fd(fd);
    set_name("server");
}

TcpServerDriver::~TcpServerDriver()
{
}

int TcpServerDriver::init(TcpConnConfig * configPtr, TcpPoller * pollerPtr)
{
    m_configPtr = dynamic_cast<TcpConfig*>(configPtr);
    m_pollerPtr = dynamic_cast<TcpEPollerMgr*>(configPtr);
    return 0;
}

int TcpServerDriver::exit()
{
    return 0;
}

int TcpServerDriver::handler_recv()
{
    int rc = 0;
    struct sockaddr_storage from;
    socklen_t namesize;
    int fd = -1;
    TcpClientDriverPtr client_handlerPtr;
    TcpConnDriverPtr handlerPtr;

    namesize = sizeof(from);
    fd = accept(get_fd(), (struct sockaddr *) &from, &namesize);
    if (fd < 0) {
        rc = -errno;
        goto l_out;
    }

    rc = set_keepalive(fd);
    rc = set_nodelay(fd);
    set_non_blocking(fd);

    client_handlerPtr = std::make_shared<TcpClientDriver>(fd);
    handlerPtr = std::dynamic_pointer_cast<TcpConnDriver>(client_handlerPtr);
    m_pollerPtr->add_event(EPOLLIN, handlerPtr);
    rc = 0;

l_out:
    return rc;
}

int TcpServerDriver::handler_send()
{
    return 0;
}

int TcpServerDriver::sched_handler()
{
    return 0;
}

