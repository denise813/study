#ifndef TCP_CONFIG_H
#define TCP_CONFIG_H


#include <memory>


using namespace std;


typedef struct tcp_configure
{
    int getPort() { return m_port; }
    std::string getHost() { return m_host; }
private:
    int m_port = 9006;
    int m_maxConnum = 100;
    std::string m_host;
}tcp_configure_t;


class TcpConfigure
{
public:
    TcpConfigure() = default;
    ~TcpConfigure() = default;

public:
    int getPort() { return m_config.m_port; }
    int getConnMax() { return m_config.m_maxConnum; }
private:
    tcp_configure_t m_config;
};


typedef std::shared_ptr<TcpConfigure> TcpConfigurePtr;


#endif