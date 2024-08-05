#ifndef TCP_CONN_HANDLER_H
#define TCP_CONN_HANDLER_H


#include <memory>


using namespace std;


#define TCP_READ_BUFFER_SIZE 2048
#define TCP_WRITE_BUFFER_SIZE 2048


enum TCP_EVENT_TYPE
{
    TCP_EVENT_TYPE_RECV_NODATA = 1,
    TCP_EVENT_TYPE_RECV_BUFFER_FULL,
    TCP_EVENT_TYPE_SEND_NODATA,
    TCP_EVENT_TYPE_SEND_BUFFER_FULL,
    TCP_EVENT_TYPE_SEND_BUFFER_NOMEM,
    TCP_EVENT_TYPE_RECV_CONTINUE,
    TCP_EVENT_TYPE_SEND_FINISH,
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
public:
    char * getReadbuffer() { return m_read_buf; }
    int getReadIdx() { return m_read_idx; }
    size_t getReadbufferSize() { return TCP_READ_BUFFER_SIZE; }
    void setReadIdx(int idx) { m_read_idx = idx; }
public:
    char * getWritebuffer() { return m_write_buf; }
    size_t getWritebufferSize() { return TCP_WRITE_BUFFER_SIZE; }
    int getWriteIdx() { return m_write_idx; }
    void setWriteIdx(int idx) { m_write_idx = idx; }

    int setSendIOV(struct iovec * iovec,size_t size) {
        int rc = 0;
        if (size > 2) {
            rc = -1;
            return rc;
        }
        memcpy(&m_send_iovec, iovec, sizeof(struct iovec) * size);
        m_send_iovec_count = size;
    }
    size_t getSendIOVSize() { return m_send_iovec_count; }
    struct iovec * getSendIOV() {return &m_send_iovec; }

    void setSendedSize(ssize_t size) { m_bytes_sended = size; }
    ssize_t getSendedSize() { return m_bytes_sended; }
    void setSendRestSize(ssize_t size) { m_bytes_send_need = size; }
    ssize_t getSendRestSize() { return m_bytes_send_need; }
    
protected:
    int m_fd;
    char m_read_buf[TCP_READ_BUFFER_SIZE];
    long m_read_idx;
    long m_checked_idx;

    char m_write_buf[TCP_WRITE_BUFFER_SIZE];
    int m_write_idx;
    struct iovec m_send_iovec[2];
    size_t m_send_iovec_count;
    ssize_t m_bytes_send_need;
    ssize_t m_bytes_sended;
};


typedef std::shared_ptr<TcpConnContext> TcpConnContextPtr;


class TcpConnHandler : public TcpConnManager
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