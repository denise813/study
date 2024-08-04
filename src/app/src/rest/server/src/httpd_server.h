#ifndef HTTPD_SERVER_H
#define HTTPD_SERVER_H



#include <cassert>
#include <memory>

#include "httpd_config.h"
#include "tcp_server.h"


class HTTPDServer : public TcpServer
{
public:
    HTTPDServer(HTTPDConfigure* configPtr, TcpConnHandler* handlerPtr);
    ~HTTPDServer();
public:
    int init();
    int start();
    int loop();
    int stop();
    int exit();
public:
    int setTcpServer();
private:
    HTTPDConfigure * m_configPtr;
};


typedef std::shared_ptr<TcpServer> TcpServerPtr;

#endif
