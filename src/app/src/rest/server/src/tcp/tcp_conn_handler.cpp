#include "tcp_conn_handler.h"


bool TcpConnHandler::process_recv(TcpConnContextPtr contextPtr)
{
    if (contextPtr->getmReadIdx() >= HTTPD_READ_BUFFER_SIZE) {
        return 0;
    }
    int bytes_read = 0;

    while (true) {
        bytes_read = recv(contextPtr->getFd(), contextPtr->getReadBuf(), contextPtr->getReadLenght(), 0);
        if (bytes_read == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                break;
            return errno;;
        } else if (bytes_read == 0) {
            return false;
        }
       contextPtr->setReadIdx(contextPtr->getmReadIdx() + bytes_read);;
    }
    return true;
}

bool TcpConnHandler::process_write(TcpConnContextPtr contextPtr)
{
    int temp = 0;

    while (1) {
        temp = writev(contextPtr->getFd(), m_iv, m_iv_count);

        if (temp < 0) {
            if (errno == EAGAIN) {
                modfd(m_epollfd, m_sockfd, EPOLLOUT, m_TRIGMode);
                return true;
            }
            unmap();
            return false;
        }

        bytes_have_send += temp;
        bytes_to_send -= temp;
        if (bytes_have_send >= m_iv[0].iov_len) {
            m_iv[0].iov_len = 0;
            m_iv[1].iov_base = m_file_address + (bytes_have_send - m_write_idx);
            m_iv[1].iov_len = bytes_to_send;
        } else {
            m_iv[0].iov_base = m_write_buf + bytes_have_send;
            m_iv[0].iov_len = m_iv[0].iov_len - bytes_have_send;
        }

        if (bytes_to_send <= 0) {
            unmap();
            modfd(m_epollfd, m_sockfd, EPOLLIN, m_TRIGMode);

            if (m_linger) {
                init();
                return true;
            } else {
                return false;
            }
        }
    }
}


