#include "httpd_server.h"


const int MAX_FD = 65536;           //最大文件描述符
const int MAX_EVENT_NUMBER = 10000; //最大事件数
const int TIMESLOT = 5;             //最小超时单位


static int g_signal_handle_pipefd[2] = {-1, -1};


void default_signal_handler(int sig)
{
    int save_errno = errno;
    int msg = sig;
    send(g_signal_handle_pipefd[1], (char *)&msg, 1, 0);
    errno = save_errno;
}


TcpServer::TcpServer(TcpConfigure* configPtr)
{
    m_configPtr = configPtr;
}

TcpServer::~TcpServer()
{
    //m_configPtr.reset();
}

int TcpServer::init()
{
    int rc = 0;
    rc = handle_event_register();
    if (rc < 0) {
        goto l_err;
    }

    rc = handle_net_register();
    if (rc < 0) {
        goto l_err_event;
    }

    (void)handle_timer_register();
    (void)handle_conn_register();
    (void)handle_signal_register();

l_out:
   return rc;

l_err_event:
    handle_event_unregister();
l_err:
    goto l_out;
}

int TcpServer::setConnHandler(TcpConnHandler* handlerPtr)
{
    m_handlerPtr = handlerPtr;
    m_handlerPtr->setEventfd(m_epfd);
}

int TcpServer::exit()
{
    handle_conn_unregister();
    handle_event_unregister();
    handle_timer_unregister();
    handle_net_unregister();
    return 0;
}

int TcpServer::handle_event_register()
{
    int rc = 0;
    int epfd = SocketAPI::create_epoll_socket(5);
    if (epfd < 0) {
        rc = -errno;
        goto l_error;
    }

    rc = SocketAPI::create_pipe_socket(g_signal_handle_pipefd, 2);
    if (rc < 0) {
       rc = -errno;
       goto l_err_close_epfd;
    }
    (void)SocketAPI::add_epoll_event(epfd);
    m_epfd = epfd;
    rc = 0;

l_out:
    return rc;

l_err_close_epfd:
    SocketAPI::destrory_server_socket(epfd);
l_error:
    goto l_out;
}

int TcpServer::handle_event_unregister()
{
    if (m_epfd < 0) {
        return 0;
    }

    SocketAPI::destrory_server_socket(m_epfd);
    m_epfd = 0;
    return 0;
}

int TcpServer::handle_net_register()
{
    int rc = 0;
    int listenfd = -1;

    listenfd = SocketAPI::create_server_socket();
    if (listenfd) {
        rc = -errno;
        return rc;
    }

    (void)SocketAPI::set_server_opts(listenfd);
    rc = SocketAPI::server_bind_listen(listenfd, m_configPtr->getPort());
    if (rc < 0) {
        rc = -errno;
        goto l_err_closesocket;
    }

    (void)SocketAPI::add_epoll_event(listenfd);

    m_listenfd = listenfd;

    rc = 0;
l_out:
    return rc;

l_err_closesocket:
    SocketAPI::destrory_socket(listenfd);
    goto l_out;
}

int TcpServer::handle_net_unregister()
{
    if (m_listenfd < 0) {
        return 0;
    }

    SocketAPI::destrory_server_socket(m_listenfd);
    m_listenfd = -1;
    return 0;
}

void TcpServer::handle_timer_register()
{
    (void)Utils::set_socket_nonblocking(m_timeout_pipefds[0]);
    (void)Utils::add_event(m_epfd, m_timeout_pipefds[1]);
    m_timerPtr = std::make_shared<TcpAlrmTimer>(m_timeout_pipefds[0]);
    m_timerPtr->init();
    return 0;
}

int TcpServer::handle_conn_register()
{
    m_connManagerPtr = std::make_shared<TcpConnManager>(
                    m_epfd,
                    m_timerManagerPtr,
                    m_handlerPtr);
    
    m_connManagerPtr->init();
}

int TcpServer::handle_conn_unregister()
{
    m_connManagerPtr->exit();
    return 0;
}

int void TcpServer::handle_timer_unregister()
{
    m_timerPtr->exit();
}

int TcpServer::handle_signal_register()
{
    (void)Utils::set_socket_nonblocking(g_signal_handle_pipefd[0]);
    (void)Utils::add_event(m_epfd, g_signal_handle_pipefd[1]);
    Utils.addsig(SIGPIPE, SIG_IGN);
    //utils.addsig(SIGALRM, TcpServer::httpd_server_handle_signal, false);
    Utils.addsig(SIGTERM, TcpServer::default_signal_handler, false);
    return 0;
}

int TcpServer::handle_accept_event(int listenfd)
{
    int rc = 0;
    struct sockaddr_in client_address;
    socklen_t client_addrlength = sizeof(client_address);
    int clientfd = -1;
 
    while (1) {
        clientfd = accept(listenfd, (struct sockaddr *)&client_address, &client_addrlength);
        if (clientfd < 0) {
            clientfd = -errno;
            LOG_ERROR("%s:errno is:%d", "accept error", clientfd);
            break;
        }
        if (m_connManagerPtr->getConnNum() >= m_configPtr->getConnMax())
        {
            SocketAPI::destrory_server_socket(clientfd);
            rc = -EBUSY;
            LOG_ERROR("%s", "Internal server busy");
            break;
        }
        handle_connect_event(clientfd);
    }
l_out:
    return rc;
}

int TcpServer::handle_connect_event(int fd)
{
    m_connManagerPtr->create_conn(fd);

    return 0;
}

int TcpServer::handle_close_event(int fd)
{
    m_connManagerPtr->do_close(fd);
    return 0;
}

int TcpServer::handle_single_event(int fd)
{
    m_stope = true;
    return 0;
}

int TcpServer::handle_timeout_event(int fd)
{
    m_connManagerPtr->do_timeout(fd);
    return 0;
}

int TcpServer::handle_client_recv(int fd)
{
    m_connManagerPtr->do_request(fd);
    return 0;
}

void TcpServer::handel_client_send(int fd)
{
    m_connManagerPtr->do_respones(fd)
    return 0;
}

void TcpServer::handle_loop_event()
{
    bool timeout = false;
    bool stop_server = false;
    int sockfd = -1;

    while (!stop_server) {
        int number = epoll_wait(m_epollfd, events, MAX_EVENT_NUMBER, -1);
        if (number < 0 && errno != EINTR) {
            LOG_ERROR("%s", "epoll failure");
            break;
        }

        for (int i = 0; i < number; i++) {
            sockfd = events[i].data.fd;
            if (sockfd == m_listenfd) {
                handle_accept_event(sockfd);
            } else if (events[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
                handle_close_event(sockfd);
            } else if ((sockfd == g_signal_handle_pipefd[0]) && (events[i].events & EPOLLIN)) {
                handle_single_event(sockfd, stop_server);
            } else if ((sockfd == m_timeout_pipefds[0]) && (events[i].events & EPOLLIN)) {
                handle_timeout_event(sockfd, stop_server);
            } else if (events[i].events & EPOLLIN) {
                handle_client_recv(sockfd);
            } else if (events[i].events & EPOLLOUT) {
                handel_client_send(sockfd);
            }
        }
    }
}
