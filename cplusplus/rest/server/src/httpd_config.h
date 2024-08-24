#ifndef HTTPD_CONFIG_H
#define HTTPD_CONFIG_H


#include <memory>
#include "tcp/tcp_config.h"


using namespace std;


typedef struct httpd_configure
{
    int m_TRIGMode = 0; //触发组合模式
    int m_LISTENTrigmode = 0;     //listenfd触发模式
    int m_CONNTrigmode = 0;       //connfd触发模式
    int m_OPT_LINGER = 0;     //优雅关闭链接
    int m_close_log = 0;
    int m_actor_model = 0;    //并发模型选择
    int m_conn_num = 1;
}httpd_configure_t;


class HTTPDConfigure
{
public:
    HTTPDConfigure() = default;
    ~HTTPDConfigure() = default;
public:
    void parse(int argc, char*argv[]);
public:
    TcpConfigure* getTcpConfigure() { return m_tcpConfigPtr.get(); }
    int getPort() { return m_tcpConfigPtr->getgetPort(); }
    
    int getMode() { return m_http_config.m_actor_model; }
    int getConnMax() { return m_http_config.m_conn_num; }
private:
    TcpConfigurePtr m_tcpConfigPtr;
    httpd_configure_t m_config;
};


typedef std::shared_ptr<HTTPDConfigure> HTTPDConfigurePtr;


#endif