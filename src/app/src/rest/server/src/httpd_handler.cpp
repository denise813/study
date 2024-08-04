#include "httpd_handler.h"


HTTPDHandler::HTTPDHandler(HTTPDConfigure* configPtr)
{
    m_configPtr = configPtr;
}

HTTPDHandler::~HTTPDHandler()
{
}

TcpConnContextPtr HTTPDHandler::createContext(int fd)
{
    HTTPDConnContextPtr httpConnContextPtr = std::make_shared<HTTPDConnContext>(fd);
    TcpConnContextPtr contextPtr =  std::dynamic_pointer_case<TcpConnContext>(httpConnContextPtr);;
    return contextPtr;
}

HTTP_PARSTER_CODE HTTPDHandler::parse_request_line(char *text, HTTPDConnContextPtr httpConnContextPtr)
{
    httpConnContextPtr->m_url = strpbrk(text, " \t");
    if (!httpConnContextPtr->m_url) {
        return HTTP_PARSTER_CODE_BAD_REQUEST;
    }
    *httpConnContextPtr->m_url++ = '\0';
    char *method = text;
    if (strcasecmp(method, "GET") == 0) {
        httpConnContextPtr->m_method = HTTP_METHOD_GET;
   } else if (strcasecmp(method, "POST") == 0) {
        httpConnContextPtr->m_method = POST;
        //cgi = 1;
    } else {
        return HTTP_PARSTER_CODE_BAD_REQUEST;
    }

    // verion
    httpConnContextPtr->m_url += strspn(httpConnContextPtr->m_url, " \t");
    httpConnContextPtr->m_version = strpbrk(httpConnContextPtr->m_url, " \t");
    if (!httpConnContextPtr->m_version) {
        return HTTP_PARSTER_CODE_BAD_REQUEST;
    }
    *httpConnContextPtr->m_version++ = '\0';
    httpConnContextPtr->m_version += strspn(httpConnContextPtr->m_version, " \t");
    if (strcasecmp(httpConnContextPtr->m_version, "HTTP/1.1") != 0) {
        return HTTP_PARSTER_CODE_BAD_REQUEST;
    }

    if (strncasecmp(httpConnContextPtr->m_url, "http://", 7) == 0)
    {
        httpConnContextPtr->m_url += 7;
        httpConnContextPtr->m_url = strchr(httpConnContextPtr->m_url, '/');
    }

    if (strncasecmp(m_url, "https://", 8) == 0)
    {
       httpConnContextPtr-> m_url += 8;
       httpConnContextPtr->m_url = strchr(httpConnContextPtr->m_url, '/');
    }

    if (!httpConnContextPtr->m_url || httpConnContextPtr->m_url[0] != '/') {
        return HTTP_PARSTER_CODE_BAD_REQUEST;
    }
    //当url为/时，显示判断界面
    if (strlen(httpConnContextPtr->m_url) == 1) {
        //strcat(m_url, "judge.html");
    }
    httpConnContextPtr->setCheckState(HTTP_CHECK_STATE_HEADER);
    return HTTP_PARSTER_CODE_NO_REQUEST;
}

int HTTPDHandler::parse_headers(char *text, HTTPDConnContextPtr httpConnContextPtr)
{
    if (text[0] == '\0') {
        if (httpConnContextPtr->getContentLenght() != 0) {
            httpConnContextPtr->setCheckState(HTTP_CHECK_STATE_CONTENT);
            return HTTP_PARSTER_CODE_NO_REQUEST;
        }

        return HTTP_PARSTER_CODE_GET_REQUEST;
    } else if (strncasecmp(text, "Connection:", 11) == 0) {
        text += 11;
        text += strspn(text, " \t");
        if (strcasecmp(text, "keep-alive") == 0) {
            httpConnContextPtr->m_linger = true;
        }
    } else if (strncasecmp(text, "Content-length:", 15) == 0) {
        text += 15;
        text += strspn(text, " \t");
        httpConnContextPtr->setContentLenght(atol(text));
    }
    else if (strncasecmp(text, "Host:", 5) == 0)
    {
        text += 5;
        text += strspn(text, " \t");
        httpConnContextPtr->setHost(text);
    }
    else
    {
        //LOG_INFO("oop!unknow header: %s", text);
    }
    return HTTP_PARSTER_CODE_NO_REQUEST;
}

int HTTPDHandler::parse_content(char *text, HTTPDConnContextPtr httpConnContextPtr)
{
     if (httpConnContextPtr->getReadIdx() >= 
                    httpConnContextPtr->getContentLength() + httpConnContextPtr->getCheckedIdx())
    {
        text[httpConnContextPtr->getContentLength()] = '\0';
        //POST请求中最后为输入的用户名和密码
        httpConnContextPtr->setString(text);
        return GET_REQUEST;
    }
    return NO_REQUEST;
    return 0;
}

int HTTPDHandler::do_request(HTTPDConnContextPtr httpConnContextPtr)
{
    return 0;
}

int HTTPDHandler::handle_request(TcpConnContextPtr contextPtr)
{
    int rc = 0;
    HTTPDConnContextPtr httpConnContextPtr = std::dynamic_pointer_case<HTTPDConnContext>(contextPtr);
    HTTPD_LINE_STATUS line_status = HTTP_LINE_STATUS_OK;
    httpConnContextPtr->setRecode(HTTP_PARSTER_CODE_NO_REQUEST);
    char *text = nullptr;

    while ((httpConnContextPtr->getCheckState() == HTTP_CHECK_STATE_REQUESTLINE && line_status == LINE_OK) ||
                    ((line_status = parse_line()) == LINE_OK))
    {
        text = httpConnContextPtr->m_read_buf + m_start_line;
        m_start_line = m_checked_idx;
        LOG_INFO("%s", text);
        switch (httpConnContextPtr->getCheckState())
        {
        case HTTP_CHECK_STATE_REQUESTLINE:
        {
            rc = parse_request_line(text, httpConnContextPtr);
            if (rc == HTTP_PARSTER_CODE_NO_REQUEST) {
                return TCP_EVENT_TYPE_RECV_CONTINUE;
            }
            break;
        }
        case HTTP_CHECK_STATE_HEADER:
        {
            rc = parse_headers(text, httpConnContextPtr);
            if (rc == HTTP_PARSTER_CODE_BAD_REQUEST) {
                httpConnContextPtr->setCode(HTTP_PARSTER_CODE_BAD_REQUEST);
            } else if (rc == HTTP_PARSTER_CODE_GET_REQUEST) {
                return do_request(httpConnContextPtr);
            }
            break;
        }
        case HTTP_CHECK_STATE_CONTENT:
        {
            rc = parse_content(text, httpConnContextPtr);
            if (rc == HTTP_PARSTER_CODE_GET_REQUEST) {
                return do_request(httpConnContextPtr);
            }
            line_status = HTTP_LINE_STATUS_OPEN;
            break;
        }
        default:
            return HTTP_PARSTER_CODE_INTERNAL_ERROR;
        }
    }
    return TCP_EVENT_TYPE_RECV_CONTINUE;
}

bool HTTPDHandler::add_response(const char *format, ...)
{
    if (m_write_idx >= WRITE_BUFFER_SIZE)
        return false;
    va_list arg_list;
    va_start(arg_list, format);
    int len = vsnprintf(m_write_buf + m_write_idx, WRITE_BUFFER_SIZE - 1 - m_write_idx, format, arg_list);
    if (len >= (WRITE_BUFFER_SIZE - 1 - m_write_idx))
    {
        va_end(arg_list);
        return false;
    }
    m_write_idx += len;
    va_end(arg_list);

    return true;
}


bool HTTPDHandler::add_status_line(int status, const char *title)
{
    return add_response("%s %d %s\r\n", "HTTP/1.1", status, title);
}

bool HTTPDHandler::add_content_length(int content_len)
{
    return add_response("Content-Length:%d\r\n", content_len);
}

bool HTTPDHandler::add_blank_line()
{
    return add_response("%s", "\r\n");
}

bool HTTPDHandler::add_headers(int content_len)
{
    return add_content_length(content_len) && add_linger() &&
           add_blank_line();
}

bool HTTPDHandler::add_content(const char *content)
{
    return add_response("%s", content);
}

int HTTPDHandler::handle_response(TcpConnContextPtr contextPtr)
{
    int rc = 0;
    HTTPDConnContextPtr httpConnContextPtr = std::dynamic_pointer_case<HTTPDConnContext>(contextPtr);
    switch (httpConnContextPtr->getRecode()) {
        case HTTP_PARSTER_CODE_INTERNAL_ERROR: {
            add_status_line(500, error_500_title);
            add_headers(strlen(error_500_form));
            if (!add_content(error_500_form))
                return false;
            break;
        } case HTTP_PARSTER_CODE_BAD_REQUEST: {
            add_status_line(404, error_404_title);
            add_headers(strlen(error_404_form));
            if (!add_content(error_404_form))
                return false;
            break;
        } case HTTP_PARSTER_CODE_FORBIDDEN_REQUEST: {
            add_status_line(403, error_403_title);
            add_headers(strlen(error_403_form));
            if (!add_content(error_403_form))
                return false;
            break;
        } case HTTP_PARSTER_CODE_FILE_REQUEST: {
            add_status_line(200, ok_200_title);
            if (m_file_stat.st_size != 0) {
                add_headers(m_file_stat.st_size);
                m_iv[0].iov_base = m_write_buf;
                m_iv[0].iov_len = m_write_idx;
                m_iv[1].iov_base = m_file_address;
                m_iv[1].iov_len = m_file_stat.st_size;
                m_iv_count = 2;
                bytes_to_send = m_write_idx + m_file_stat.st_size;
                return true;
            } else {
                const char *ok_string = "<html><body></body></html>";
                add_headers(strlen(ok_string));
                if (!add_content(ok_string))
                    return false;
            }
        }
        default:
            return false;
    }
    m_iv[0].iov_base = m_write_buf;
    m_iv[0].iov_len = m_write_idx;
    m_iv_count = 1;
    bytes_to_send = m_write_idx;
    return 0;
}

