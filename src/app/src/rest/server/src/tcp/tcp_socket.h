#ifndef TCP_SOCKET_MANAGER_H
#define TCP_SOCKET_MANAGER_H


#include <memory>


using namespace std;


class SocketAPI
{
public:
    SocketAPI() = default;
    ~SocketAPI() = default;
public:
    static int create_server_socket();
    static int destrory_socket(int sockfd);
    static int set_server_opts(int socketfd);
    static int server_bind_listen(int sockd, int port);
    static int set_socket_nonblocking(int fd);
public:
    static int create_epoll_socket(int max);
    static int add_epoll_event(int fd);
    static int modify_epoll_event();
    static int remove_epoll_event();
public:
    static int create_pipe_socket(int *pipefds, int fdsize);
};


typedef std::shared_ptr<SocketManager> SocketManagerPtr;

#endif