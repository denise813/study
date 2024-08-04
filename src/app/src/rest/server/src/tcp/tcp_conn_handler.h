#ifndef TCP_CONN_HANDLER_H
#define TCP_CONN_HANDLER_H


#include <memory>


using namespace std;


#define TCP_READ_BUFFER_SIZE 2048
#define TCP_WRITE_BUFFER_SIZE 2048


enum TCP_EVENT_TYPE
{
    TCP_EVENT_TYPE_RECV_NODATA = 1,
    TCP_EVENT_TYPE_SEND_NODATA = 2,
    TCP_EVENT_TYPE_RECV_CONTINUE = 3,
    TCP_EVENT_TYPE_SEND_FINISH = 4,
};


class TcpConnContext
{
public:
    TcpConnContext(int fd) {
        m_fd = fd;
    }
    virtual ~TcpConnContext() = default;
public:
    int getFd() { return m_fd; }
protected:
    int m_fd;
    char m_read_buf[TCP_READ_BUFFER_SIZE];
    long m_read_idx;
    long m_checked_idx;

    char m_write_buf[TCP_WRITE_BUFFER_SIZE];
    int m_write_idx;
     struct iovec m_iv[2];
    int m_iv_count;
    int bytes_to_send;
    int bytes_have_send;
};


typedef std::shared_ptr<TcpConnContext> TcpConnContextPtr;


class TcpConnHandler
{
public:
    TcpConnHandler() = default;
    virtual ~TcpConnHandler() = default;
public:
    virtual TcpConnContextPtr createContext(int fd) = 0;
    virtual int process_read(TcpConnContextPtr contextPtr) = 0;
    virtual int process_write(TcpConnContextPtr contextPtr) = 0;
    virtual int process_recv(TcpConnContextPtr contextPtr);
    virtual int process_send(TcpConnContextPtr contextPtr);
};


typedef std::shared_ptr<TcpConnHandler> TcpConnHandlerPtr;


#endif