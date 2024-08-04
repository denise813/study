#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/epoll.h>


#include <cassert>
#include <memory>

#include "httpd_config.h"
#include "tcp_socket_manager.h"


class TcpServer
{
public:
    TcpServer(TcpConfigure* configPtr);
    ~TcpServer();
public:
    int init();
    int exit();
    int setConnHandler(TcpConnHandler* handlerPtr);
private:
    int bind_listen();
    int handle_signal_register();
private:
    int handle_accept_event(int fd);
    int handle_connect_event(int fd);
    int handle_close_event(int fd);
    int handle_single_event(int fd);
    int handle_client_recv(int fd);
    int handel_client_send(int fd);
    int handle_timeout_event(int fd);
private:
    int m_listenfd = -1;
    int m_epfd = -1;
    int m_timeout_pipefds[2] = {-1, -1};
private:
    TcpConnHandler * m_handlerPtr;
    TcpConfigure * m_configPtr;
    TcpAlrmTimer* m_timerPtr;
    TcpConnManager * m_connManagerPtr;
};


typedef std::shared_ptr<TcpServer> TcpServerPtr;

#endif
