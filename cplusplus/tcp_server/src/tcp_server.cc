#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <sys/time.h>
#include <sys/resource.h>


#include <vector>

#include "utility/string_format_tools.hpp"
#include "tcp_mgr.h"
#include "tcp_server.h"
#include "tcp_conn.h"


using namespace std;


static std::vector<TcpConnDriverPtr> g_tcp_conn_drivers;
int register_conn_driver(TcpConnDriverPtr & drvPtr)
{
    g_tcp_conn_drivers.push_back(drvPtr);
    return 0;
}


static int oom_adjust(void)
{
    int rc = 0;
    int fd = 0;
    struct stat st;
    const char *score = "-1000\n";;

    /* Avoid oom-killer */
    rc = stat("/proc/self/oom_score_adj", &st);
    if (rc == 0) {
        goto l_out;
    }

    fd = open("/proc/self/oom_adj", O_WRONLY);
    if (fd < 0) {
        rc = -errno;
         goto l_out;
    }

    rc = write(fd, score, strlen(score));
    if (rc < 0) {
        rc = -errno;
        goto l_close;
    }
    close(fd);
    rc = 0;

l_out:
    return rc;
l_close:
    close(fd);
    goto l_out;
}

static int nr_file_adjust(void)
{
    int rc = 0;
    int fd = -1;
    int max = 1024 * 1024;
    char buf[64];
    struct rlimit rlim;

    /* Avoid oom-killer */
    fd = open("/proc/sys/fs/nr_open", O_RDONLY);
    if (fd < 0) {
        rc = -errno;
        goto set_rlimit;
    }
    rc = read(fd, buf, sizeof(buf));
    if (rc < 0) {
        rc = -errno;
        goto l_close;
    }
    close(fd);
    max = atoi(buf);

set_rlimit:
    rlim.rlim_cur = rlim.rlim_max = max;
    rc = setrlimit(RLIMIT_NOFILE, &rlim);
    if (rc != 0) {
        rc = -errno;
        goto l_out;
    }
    rc = 0;

l_out:
    return rc;

l_close:
    close(fd);
    goto l_out;
}

static int lld_init(TcpConnConfig * configPtr, TcpDriverMgr * connMgrPtr)
{
    int rc = 0;
    for (auto itor : g_tcp_conn_drivers) {
        rc = itor->init(configPtr, connMgrPtr);
        if (rc < 0) {
            return rc;
        }
    }

    return 0;
}

static void lld_exit(void)
{
    int rc;
    for (auto itor : g_tcp_conn_drivers) {
        rc = itor->exit();
        if (rc < 0) {
            // nothing to do
        }
    }
}


int TcpServer::init(TcpConfig* configPtr)
{
    m_configPtr = configPtr;
    m_connMgrptr = std::shared_ptr<TcpConnMgr>();
    m_mgrPtr = std::make_shared<TcpMgr>();

    m_connMgrptr->init();
    m_mgrPtr->init(m_configPtr, m_connMgrptr.get());

    return 0;
}

int TcpServer::exit()
{
    m_mgrPtr->exit();
    m_connMgrptr->exit();

    return 0;
}

int TcpServer::start()
{
    return 0;
}

int TcpServer::loop()
{
    m_connMgrptr->loop();
    return 0;
}

int TcpServer::stop()
{
    return 0;
}


int TcpServer::create_pid_file()
{
    int fd = -1;
    int rc = 0;
    std::string file_buff;

    std::string pid_file = string_format("%s/%s.%d.pid",
                    m_configPtr->get_run_dir().c_str(),
                    m_configPtr->get_process_name().c_str(),
                    getpid());

    fd = open(pid_file.c_str(), O_RDWR | O_CREAT,
                    S_IRUSR | S_IWUSR | S_IRGRP);
    if (fd < 0) {
        rc = -errno;
        goto l_unlink;
    }

    file_buff = string_format("%d", getpid());
    rc = write(fd, file_buff.c_str(), strlen(file_buff.c_str()));
    if (rc < 0) {
        rc = -errno;
        goto l_close;
    }
    rc = 0;

l_out:
    return rc;

l_close:
    close(fd);
l_unlink:
     unlink(pid_file.c_str());
    goto l_out;
}


