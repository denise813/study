#ifndef _TCP_SERVER_H_
#define _TCP_SERVER_H_


#include <memory>

#include "tcp_mgr.h"
#include "tcp_conn.h"
#include "tcp_config.h"


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
private:
    TcpMgrPtr m_mgrPtr;
    TcpConnMgrPtr m_connMgrptr;
    TcpConfig* m_configPtr;
    int m_sock_fd = 0;
};


typedef std::shared_ptr<TcpServer> TcpServerPtr;


#endif
