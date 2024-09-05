#ifndef _TCP_MGR_H
#define _TCP_MGR_H


#include <memory>
#include "tcp_config.h"
#include "tcp_conn.h"


using namespace std;


class TcpMgr
{
public:
    TcpMgr() = default;
    virtual ~TcpMgr() = default;
public:
    int init(TcpConfig * configPtr, TcpConnMgr * connMgrPtr);
    int exit();
private:
    int create_mgr_sock();
private:
    TcpConfig* m_configPtr = nullptr;
    TcpConnMgr * m_connMgrPtr = nullptr;
    int m_sock_fd = 0;
};


typedef std::shared_ptr<TcpMgr> TcpMgrPtr;


#endif
