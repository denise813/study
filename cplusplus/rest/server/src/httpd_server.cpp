#include "httpd_server.h"


const int MAX_FD = 65536;           //最大文件描述符
const int MAX_EVENT_NUMBER = 10000; //最大事件数
const int TIMESLOT = 5;             //最小超时单位


HTTPDServer::HTTPDServer(HTTPDConfigure * configPtr)
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
}


int HTTPDServer::stop()
{
    m_serverPtr->exit();
}


