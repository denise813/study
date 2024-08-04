#ifndef TCP_CONN_MANAGER_H
#define TCP_CONN_MANAGER_H


#include <memory>


using namespace std;


class TcpConnManager
{
public:
    TcpConnManager(int epfd,
                    TcpAlrmTimerPtr timerPtr,
                    TcpConnHandlerPtr handlerPtr);
    ~TcpConnManager() = default;
public:
    int init();
    int exit();
public:
    int create_conn(int fd);
    int destroy_conn(int fd);
    int do_timeout(int fd);
    int do_request(int fd);
    int do_respones(int fd);
    int do_close(int fd);
    int getConnNum() { return m_conns.size(); }
    //int do_keekalive(int fd);
private:
    int recvEntry();
    int sendEntry();
    int getConnContext(int fd);
private:
    int m_epfd;
    TcpConnHandler * m_handler;
    TcpAlrmTimer * m_timerPtr;
    WorkerThread m_recvThread;
    WorkerThread m_sendThead;
    std::list<TcpConnContextPtr> m_conns;
    WorkerQueue<TcpConnContextPtr> m_reqs;
    WorkerQueue<TcpConnContextPtr> m_resps;
};


typedef std::shared_ptr<SocketManager> SocketManagerPtr;

#endif