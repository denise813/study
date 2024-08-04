#include "httpd_server.h"


const int MAX_FD = 65536;           //最大文件描述符
const int MAX_EVENT_NUMBER = 10000; //最大事件数
const int TIMESLOT = 5;             //最小超时单位


static int g_signal_handle_pipefd[2] = {-1, -1};


HTTPDServer::HTTPDServer(HTTPDConfigure* configPtr,
                HTTPDHandler* handlerPtr) :
                TcpServer(configPtr->getTcpConfigure())
{
    m_configPtr = configPtr;
}

HTTPDServer::~HTTPDServer()
{
}

int HTTPDServer::init()
{
    m_serverPtr = std::make_shared<TcpServer>(m_configPtr->getTcpConfigure());
}

int HTTPDServer::exit()
{
    return 0;
}

int HTTPDServer::start()
{
    m_serverPtr->init();
    m_serverPtr->setConnHandler(m_handlerPtr);
}


int HTTPDServer::stop()
{
    m_serverPtr->exit();
}

