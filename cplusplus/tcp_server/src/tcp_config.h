#ifndef _TCP_CONFIG_H_
#define _TCP_CONFIG_H_


#include <memory>

#include "tcp_conn_config.h"


using namespace std;

typedef struct tcp_config
{
    int m_port = 0;
    bool m_is_deamon = false;
    std::string m_etc_dir;
    std::string m_run_dir;
    std::string m_mgr_sock_dir;
    std::string m_process_name;
}tcp_config_t;


class TcpConfig : public TcpConnConfig
{
public:
    TcpConfig();
    ~TcpConfig();
public:
    int init();
    int exit();
public:
    int get_port() { return m_config.m_port; }
    bool is_deamon() { return m_config.m_is_deamon; }
    std::string get_etc_dir() { return m_config.m_etc_dir; }
    std::string get_run_dir() { return m_config.m_run_dir; }
    std::string get_process_name() { return m_config.m_process_name; }
    std::string get_mgr_sock_dir() { return m_config.m_mgr_sock_dir; }
private:
    tcp_config_t m_config;
};


typedef std::shared_ptr<TcpConfig> TcpConfigPtr;


#endif