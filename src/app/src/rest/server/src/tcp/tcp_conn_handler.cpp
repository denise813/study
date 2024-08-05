#include "tcp_conn_handler.h"


int TcpConnHandler::process_recv(TcpConnContextPtr contextPtr)
{
    int bytes_read = 0;
    int rc = 0;

    if (contextPtr->getReadIdx() >= contextPtr->getReadbufferSize()) {
        return TCP_EVENT_TYPE_RECV_BUFFER_FULL;
        return 0;
    }

    while (true) {
        bytes_read = recv(contextPtr->getFd(), contextPtr->getReadBuf(), contextPtr->getReadLenght(), 0);
        if (bytes_read == -1) {
            rc = -errno;
            if (rc == -EAGAIN || rc == -EWOULDBLOCK) {
                return TCP_EVENT_TYPE_RECV_CONTINUE;
            }
            return rc;
        } else if (bytes_read == 0) {
            return TCP_EVENT_TYPE_RECV_NODATA;
        }
       contextPtr->setReadIdx(contextPtr->getmReadIdx() + bytes_read);;
    }
    return 0;
}

int TcpConnHandler::process_write(TcpConnContextPtr contextPtr)
{
    ssize_t bytes_write = 0;
    int rc = 0;
    struct iovec * send_iovec = nullptr;

    while (1) {
        send_iovec = contextPtr->getSendIOV();
        bytes_write = writev(contextPtr->getFd(), send_iovec, contextPtr->getSendIOVSize());
        if (bytes_write < 0) {
            rc = -errno;
            return rc;
        } else if (bytes_write == 0) {
            return TCP_EVENT_TYPE_SEND_NODATA;
        }
        contextPtr->setSendedSize(contextPtr->getSendedSize() + bytes_write);
        contextPtr->setSendRestSize(contextPtr->getSendRestSize() - bytes_write);

        if (contextPtr->setSendedSize() >= send_iovec[0].iov_len) {
            send_iovec[0].iov_len = 0;
            send_iovec[1].iov_base = send_iovec[1].iov_base + 
                        contextPtr->getSendedSize() - send_iovec[0].iov_len);
            send_iovec[1].iov_len = contextPtr->getSendRestSize();
        } else {
            send_iovec[0].iov_base = contextPtr->getWriteBuff() + contextPtr->getSendedSize();
            send_iovec[0].iov_len = send_iovec[0].iov_len - contextPtr->getSendedSize();
        }
    }
}

