#include "httpd_server.h"


const int MAX_FD = 65536;           //最大文件描述符
const int MAX_EVENT_NUMBER = 10000; //最大事件数
const int TIMESLOT = 5;             //最小超时单位


HTTPDServer::HTTPDServer(HTTPDConfigure* configPtr)
{
    m_configPtr = configPtr;
}

HTTPDServer::~HTTPDServer()
{
    m_configPtr.reset();
}

int HTTPDServer::init()
{
    bind_listen();
    timer(m_listenfd);
    timer(m_epollfd);
   return 0;
}

int HTTPDServer::exit()
{
    return 0;
}

int HTTPDServer::bind_listen()
{
    int rc = 0;
    int listenfd = -1;
    listenfd = socket(PF_INET, SOCK_STREAM, 0);
    if (listenfd) {
        rc = -errno;
        return rc;
    }

    //优雅关闭连接
    struct linger tmp = {1, 1};
    setsockopt(listenfd, SOL_SOCKET, SO_LINGER, &tmp, sizeof(tmp));

    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(m_configPtr->getPort());

    int flag = 1;
    setsockopt(m_listenfd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));
    rc = bind(m_listenfd, (struct sockaddr *)&address, sizeof(address));
    if (rc < 0) {
        rc = -errno;
        goto l_err_closesocket;
    }

    rc = listen(listenfd, 5);
    if (rc < 0) {
        rc = -errno;
        goto l_err_closesocket;
    }

    epoll_event events[MAX_EVENT_NUMBER];
    int epfd = -1;
    epfd = epoll_create(5);
    if (epfd < 0) {
         rc = -errno;
         goto l_err_closesocket;
    }

    rc = socketpair(PF_UNIX, SOCK_STREAM, 0, m_pipefd);
    if (rc < 0) {
        rc = -errno;
        goto l_err_close_epfd;
    }

    m_listenfd = listenfd;
    m_epollfd = epfd;

    rc = 0;
l_out:
    return rc;

l_err_close_epfd;
    close(epfd);

l_err_closesocket:
    close(listenfd);
    goto l_out;

}


void HTTPDServer::timer(int fd)
{
    return 0;
}


int HTTPDServer::accept()
{
    int rc = 0;
    struct sockaddr_in client_address;
    socklen_t client_addrlength = sizeof(client_address);
    int clientfd = -1;
 
    while (1) {
        clientfd = accept(m_listenfd, (struct sockaddr *)&client_address, &client_addrlength);
        if (clientfd < 0) {
            clientfd = -errno;
            LOG_ERROR("%s:errno is:%d", "accept error", clientfd);
            break;
        }
        if (http_conn::m_user_count >= m_configPtr->getConnMax())
        {
            close(clientfd);
            rc = -EBUSY;
            LOG_ERROR("%s", "Internal server busy");
            break;
        }
    }
l_out:
    return rc;
}

int HTTPDServer::handle_signal(bool &timeout, bool &stop_server)
{
    return 0;
}

int HTTPDServer::handel_recv(int sockfd)
{
    return 0;
}

void HTTPDServer::handel_send(int sockfd)
{
   return 0;
}

void HTTPDServer::handle_loop_event()
{
    bool timeout = false;
    bool stop_server = false;

    while (!stop_server) {
        int number = epoll_wait(m_epollfd, events, MAX_EVENT_NUMBER, -1);
        if (number < 0 && errno != EINTR) {
            LOG_ERROR("%s", "epoll failure");
            break;
        }

        for (int i = 0; i < number; i++) {
            int sockfd = events[i].data.fd;

            //处理新到的客户连接
            if (sockfd == m_listenfd) {
                handle_accept_event();
            } else if (events[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
                //服务器端关闭连接，移除对应的定时器
                util_timer *timer = users_timer[sockfd].timer;
                handle_close_event(timer, sockfd);
            } else if ((sockfd == m_pipefd[0]) && (events[i].events & EPOLLIN)) {
                handle_signal(timeout, stop_server);
            } else if (events[i].events & EPOLLIN) {
                handle_client_recv(sockfd);
            } else if (events[i].events & EPOLLOUT) {
                handle_client_send(sockfd);
            }
        }
    }
}
