#include "socket_manager.h"


int SocketAPI::create_server_socket()
{
    int sockfd = socket(PF_INET, SOCK_STREAM, 0);
    return sockfd;
}

int SocketAPI::destrory_server_socket(int sockfd)
{
    close(sockfd);
    return 0;
}

int SocketAPI::set_server_opts(int sockfd)
{
//优雅关闭连接
    struct linger tmp = {1, 1};
    setsockopt(sockfd, SOL_SOCKET, SO_LINGER, &tmp, sizeof(tmp));

    int flag = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));
    return 0;
}

int SocketAPI::set_socket_nonblocking(int fd)
{
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}

int SocketAPI::server_bind_listen(int sockd, int port)
{
    int rc = 0;
    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(m_configPtr->getPort());

    rc = bind(sockd, (struct sockaddr *)&address, sizeof(address));
    if (rc < 0) {
        rc = -errno;
        goto l_err;
    }

    rc = listen(listenfd, 5);
    if (rc < 0) {
        rc = -errno;
        goto l_err;
    }
    rc = 0;

l_out:
    return rc;

l_err:
    goto l_out;
}

int SocketAPI::create_epoll_socket(int max)
{
    int epfd = -1;
    epfd = epoll_create(max);
    if (epfd < 0) {
         epfd = -errno;
         goto l_err;
    }

l_out:
    return epfd;

l_err:
    goto l_out;
}

int SocketAPI::create_pipe_socket(int *pipefds, int fdsize)
{
    int rc =0;
    rc = socketpair(PF_UNIX, SOCK_STREAM, 0, pipefds);
    if (rc < 0) {
         rc = -errno;
         goto l_err;
    }

l_out:
    return epfd;

l_err:
    goto l_out;
}

int SocketAPI::add_epoll_event(int epfd, int fd)
{
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
    // event.events = EPOLLIN | EPOLLRDHUP;
    event.events |= EPOLLONESHOT;
    epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &event);

    (void)set_socket_nonblocking(fd);

    return 0;
}

int SocketAPI::modify_epoll_event(int epfd, int fd, int event)
{
    epoll_event event;
    event.data.fd = fd;

    event.events = event | EPOLLET | EPOLLONESHOT | EPOLLRDHUP;
    //event.events = event | EPOLLONESHOT | EPOLLRDHUP;

    epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &event);
    return 0;
}

int SocketAPI::remove_epoll_event(int epfd, int fd)
{
    epoll_ctl(epfd, EPOLL_CTL_DEL, fd);
    close(fd);
    return 0;
}
