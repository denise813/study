#ifndef _MGR_H
#define _MGR_H


#include <memory>
#include "config.h"
#include "epoller.h"


using namespace std;


class TcpMgr
{
public:
    TcpMgr() = default;
    virtual ~TcpMgr() = default;
public:
    int init(TcpConfig * configPtr, TcpEPollerMgr  * pollerPtr);
    int exit();
private:
    int create_mgr_sock();
private:
    TcpConfig* m_configPtr = nullptr;
    TcpEPollerMgr * m_pollerPtr = nullptr;
    int m_sock_fd = 0;
};


typedef std::shared_ptr<TcpMgr> TcpMgrPtr;


#endif
