#ifndef _LOG_H_
#define _LOG_H_


#include "logging/logging.h"


using namespace std;


class TcpLoggerMgr
{
public:
    TcpLoggerMgr() = default;
    virtual ~TcpLoggerMgr() = default;
public:
    int init(std::string process_name) {
        m_process_name = process_name;
        return 0;
    }
    int exit();
private:
    std::string m_process_name;
};



typedef std::shared_ptr<TcpLoggerMgr> TcpLoggerMgrPtr;

#endif
