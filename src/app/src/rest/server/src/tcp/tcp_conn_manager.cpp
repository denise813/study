#include "conn_manager.h"


TcpConnManager::TcpConnManager(int epfd,TcpAlrmTimerPtr timerPtr)
{
    m_epfd = epfd;
    m_timerPtr = timerPtr;
}

TcpConnManager::~TcpConnManager()
{
    return 0;
}

int TcpConnManager::init()
{
    return 0;
}

int TcpConnManager::exit()
{
    return 0;
}

int TcpConnManager::create_conn(int fd)
{
    save_conn(fd);
    time_t cur = time(NULL);
    m_timerPtr->createEvent(fd, 30);
    SocketAPI::modify_epoll_event(m_epfd, fd, EPOLLIN);
    TcpConnContextPtr conncontenxtPtr = m_handler->createContext(fd);
    m_conns.push_back(fd, conncontenxtPtr);
}

int TcpConnManager::get_conn(int fd)
{
    TcpConnContextPtr conncontenxtPtr = m_conns.find(fd);
    
}

int TcpConnManager::destroy_conn(int fd)
{
    m_conns.remove(fd);
    return 0;
}

int TcpConnManager::do_request(int fd)
{
    TcpConnContextPtr conncontenxtPtr = get_conn(fd);
    m_reqs.addEntry(conncontenxtPtr);
    //SocketAPI::modify_epoll_event(m_epfd, fd, EPOLLOUT);
}

int TcpConnManager::do_respones(int fd)
{
    TcpConnContextPtr conncontenxtPtr = get_conn(fd);
    m_resps.addEntry(conncontenxtPtr);
    //m_timerManagerPtr->remove(fd);
    //SocketAPI::remove_epoll_event(fd);
}

int TcpConnManager::do_timeout(int fd)
{
    
}

int TcpConnManager::do_close(int fd)
{
    m_timerManagerPtr->remove(fd);
    SocketAPI::remove_epoll_event(fd);
}

int TcpConnManager::recvEntry()
{
    int rc = 0;
    char * out = nullptr;
    while(!m_recvThread.isStop()) {
        TcpConnContextPtr contextPtr;
        rc = m_reps.getEntry(contextPtr);
        if(rc < 0) {
            m_recvThread.wait();
            continue;
        }

        while(1) {
            m_timerPtr->updateEvent(contextPtr->getFd());
            rc = m_handler->process_recv(contextPtr);
            if (rc < 0){
                break;
            }
            if (rc == TCP_EVENT_TYPE_RECV_NODATA) {
                break;
            }
            m_timerPtr->updateEvent(contextPtr->getFd());
            rc = m_handler->process_request(contextPtr);
            if (rc == TCP_EVENT_TYPE_RECV_CONTINUE) {
                SocketAPI::modify_epoll_event(m_epfd, contextPtr->getFd(), EPOLLIN);
                continue;
            }
            m_timerPtr->setKeepAlive(contextPtr->getFd());
        }

    }
    return 0;
}

int TcpConnManager::sendEntry()
{
    int rc = 0;
    while(!m_sendThead.isStop()) {
        TcpConnContextPtr contextPtr;
        rc = m_resps.getEntry(contextPtr);
        if(rc < 0) {
            m_sendThead.wait();
            continue;
        }
        while(1) {
            rc = m_handler->process_write(contextPtr);
            if (rc == TCP_EVENT_TYPE_SEND_FINISH){
                SocketAPI::modify_epoll_event(m_epfd, contextPtr->getFd(), EPOLLOUT);
            }
            rc = m_handler->process_send(contextPtr);
            if (rc < 0){
                break;
            }
            if (rc == TCP_EVENT_TYPE_RECV_NODATA) {
                SocketAPI::modify_epoll_event(m_epfd, contextPtr->getFd(), EPOLLIN);
                break;
            }
        }
    }
    return 0;
}


