#ifndef _SESSION_H_
#define _SESSION_H_


#include <memory>
#include "conn_driver.h"


class TcpSession
{
public:
    TcpSession(TcpConnDriverPtr & drvPtr);
    ~TcpSession();
private:
    TcpConnDriverPtr m_drvPtr;
};


typedef std::shared_ptr<TcpSession> TcpSessionPtr;


#endif
