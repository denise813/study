#ifndef HTTPD_SERVER_H
#define HTTPD_SERVER_H



#include <cassert>
#include <memory>

#include "httpd_config.h"
#include "tcp_server.h"


class HTTPDServer
{
public:
    HTTPDServer(HTTPDConfigure * configPtr);
    ~HTTPDServer();
public:
    int init();
    int start();
    int loop();
    int stop();
    int exit();
private:
    HTTPDConfigure * m_configPtr;
    std::shared_ptr<TcpServer> m_serverPtr;
};


typedef std::shared_ptr<HTTPDServer> HTTPDServerPtr;

#endif
