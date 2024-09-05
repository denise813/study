#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/un.h>

#include <vector>
#include "utility/string_format_tools.hpp"

#include "mgr_conn_driver.h"
#include "mgr.h"


using namespace std;


int TcpMgr::init(TcpConfig * configPtr, TcpEPollerMgr * pollerPtr)
{
    int rc = 0;

    m_configPtr = configPtr;
    m_pollerPtr = pollerPtr;
    rc = create_mgr_sock();
    if (rc < 0) {
        goto l_out;
    }
    rc = 0;

l_out:
    return rc;
}

int TcpMgr::exit()
{
    return 0;
}

int TcpMgr::create_mgr_sock()
{
    int rc = 0;
    int lock_fd = -1;
    int fd = -1;
    std::string mgr_sock_path;
    struct sockaddr_un addr;
    TcpMgrDriverPtr handlerPtr;
    TcpConnDriverPtr drvPtr;

    std::string lock_file = string_format("%s/%s.%d.lock",
                    m_configPtr->get_etc_dir().c_str(),
                    m_configPtr->get_process_name().c_str(),
                    m_configPtr->get_port());
    lock_fd = open(lock_file.c_str(),
                    O_WRONLY | O_CREAT,
                    S_IRUSR | S_IWUSR |
                    S_IRGRP | S_IROTH);
    if (lock_fd < 0) {
        rc = -errno;
        goto l_out;
    }

    if (lockf(lock_fd, F_TLOCK, 1) < 0) {
        rc = -errno;
        goto l_close_lock_fd;
    }

    fd = socket(AF_LOCAL, SOCK_STREAM, 0);
    if (fd < 0) {
        rc = -errno;
        goto l_close_lock_fd;
    }
    mgr_sock_path = string_format("%s/%s.%d",
                    m_configPtr->get_mgr_sock_dir().c_str(),
                    m_configPtr->get_process_name().c_str(),
                    m_configPtr->get_port());
    unlink(mgr_sock_path.c_str());
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_LOCAL;
    strcpy(addr.sun_path, mgr_sock_path.c_str());
    rc = bind(fd, (struct sockaddr *) &addr, sizeof(addr));
    if (rc < 0) {
        rc = -errno;
        goto l_close_fd;
    }

    rc = listen(fd, 32);
    if (rc < 0) {
        rc = -errno;
        goto l_close_fd;
    }

    handlerPtr = std::make_shared<TcpMgrDriver>(fd);
    drvPtr = std::dynamic_pointer_cast<TcpConnDriver>(handlerPtr);
    rc = m_pollerPtr->add_event(EPOLLIN, drvPtr);
    if (rc < 0) {
        goto l_close_fd;
    }
    m_sock_fd = fd;

l_out:
    return rc;

l_close_fd:
    close(fd);
l_close_lock_fd:
    close(lock_fd);
    goto l_out;
}

