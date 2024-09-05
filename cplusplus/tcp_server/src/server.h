#ifndef _SERVER_H_
#define _SERVER_H_


#include <memory>

#include "mgr.h"
#include "epoller.h"
#include "config.h"


using namespace std;


class TcpServer
{
public:
    TcpServer() = default;
    virtual ~TcpServer() = default;
public:
    int init(TcpConfig* configPtr);
    int start();
    int loop();
    int stop();
    int exit();
private:
    int create_pid_file();
    int bind_listen();
private:
    TcpMgrPtr m_mgrPtr;
    TcpEPollerMgrPtr m_pollerPtr;
    TcpConfig* m_configPtr;
    int m_sock_fd = 0;
};


typedef std::shared_ptr<TcpServer> TcpServerPtr;


#endif
