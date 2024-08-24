#include "httpd_handler.h"



int HTTPDConnContext::do_process()
{
     int rc = 0;
     HTTPDConnContextPtr httpConnContextPtr = std::dynamic_pointer_case<HTTPDConnContext>(contextPtr);
    rc = handle_request(httpConnContextPtr);
    if (rc == TCP_BUFFER_EVENT_TYPE_RECV) {
        
    }
    rc = do_request(httpConnContextPtr);
    rc = handle_response(httpConnContextPtr);
    return 0;
}


HTTPDConnHandler::HTTPDConnHandler(HTTPDConfigure* configPtr)
{
    m_configPtr = configPtr;
}

HTTPDConnHandler::~HTTPDConnHandler()
{
}

TcpConnContextPtr HTTPDConnHandler::createContext(int fd)
{
    HTTPDConnContextPtr httpConnContextPtr = std::make_shared<HTTPDConnContext>(fd);
    TcpConnContextPtr contextPtr =  std::dynamic_pointer_case<TcpConnContext>(httpConnContextPtr);;
    return contextPtr;
}

HTTP_PARSTER_CODE HTTPDConnHandler::parse_request_line(char *text, HTTPDConnContextPtr httpConnContextPtr)
{
    
    httpConnContextPtr->m_url = strpbrk(text, " \t");
    if (!httpConnContextPtr->m_url) {
        return HTTP_PARSTER_CODE_BAD_REQUEST;
    }
    *httpConnContextPtr->m_url++ = '\0';
    char *method = text;
    if (strcasecmp(method, "GET") == 0) {
        httpConnContextPtr->m_req.setMethod(HTTP_METHOD_GET);
   } else if (strcasecmp(method, "POST") == 0) {
        httpConnContextPtr->m_req.setMethod(HTTP_METHOD_POST);
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

int HTTPDConnHandler::parse_headers(char *text, HTTPDConnContextPtr httpConnContextPtr)
{
    if (text[0] == '\0') {
        if (httpConnContextPtr->m_req.getContentLenght() != 0) {
            httpConnContextPtr->setCheckState(HTTP_CHECK_STATE_CONTENT);
            return HTTP_PARSTER_CODE_NO_REQUEST;
        }

        return HTTP_PARSTER_CODE_GET_REQUEST;
    } else if (strncasecmp(text, "Connection:", 11) == 0) {
        text += 11;
        text += strspn(text, " \t");
        if (strcasecmp(text, "keep-alive") == 0) {
            httpConnContextPtr->setlinger(true);
        }
    } else if (strncasecmp(text, "Content-length:", 15) == 0) {
        text += 15;
        text += strspn(text, " \t");
        httpConnContextPtr->m_req.setContentLenght(atol(text));
    }
    else if (strncasecmp(text, "Host:", 5) == 0)
    {
        text += 5;
        text += strspn(text, " \t");
        httpConnContextPtr->m_req.setHost(text);
    }
    else
    {
        //LOG_INFO("oop!unknow header: %s", text);
    }
    return HTTP_PARSTER_CODE_NO_REQUEST;
}

int HTTPDConnHandler::parse_content(char *text, HTTPDConnContextPtr httpConnContextPtr)
{
     if (httpConnContextPtr->getReadIdx() >= 
                    httpConnContextPtr->getContentLength() + httpConnContextPtr->getCheckedIdx())
    {
        text[httpConnContextPtr->getContentLength()] = '\0';
        //POST请求中最后为输入的用户名和密码
        httpConnContextPtr->setString(text);
        return HTTP_PARSTER_CODE_GET_REQUEST;
    }
    return HTTP_PARSTER_CODE_NO_REQUEST;
}

int HTTPDConnHandler::do_request(HTTPDConnContextPtr httpConnContextPtr)
{
    int rc = 0;
    rc = m_controller->process(&httpConnContextPtr->m_req, &httpConnContextPtr->m_resp);
    if (rc < 0) {
        httpConnContextPtr->setRccode(HTTP_RC_CODE_INTERNAL_ERROR);
        return rc;
    }
    httpConnContextPtr->setRccode(rc);
    
    //httpConnContextPtr->setRccode(HTTP_PARSTER_CODE_INTERNAL_ERROR);
    return 0;
}

int HTTPDConnHandler::handle_request(HTTPDConnContextPtr contextPtr)
{
    int rc = 0;
    HTTPD_LINE_STATUS line_status = HTTP_LINE_STATUS_OK;
    httpConnContextPtr->setRecode(HTTP_PARSTER_CODE_NO_REQUEST);
    char *text = nullptr;

    while ((httpConnContextPtr->getCheckState() == HTTP_CHECK_STATE_REQUESTLINE &&
                                    line_status == HTTP_LINE_STATUS_OK) ||
                    ((line_status = parse_line()) == HTTP_LINE_STATUS_OK))
    {
        text = httpConnContextPtr->getReadBuff() + httpConnContextPtr->getStartLine();
       httpConnContextPtr->setStartLine(contextPtr->getCheckedidx());

        switch (httpConnContextPtr->getCheckState())
        {
        case HTTP_CHECK_STATE_REQUESTLINE:
        {
            rc = parse_request_line(text, httpConnContextPtr);
            if (rc == HTTP_PARSTER_CODE_NO_REQUEST) {
                return TCP_BUFFER_EVENT_TYPE_RECV;
            }
            break;
        }
        case HTTP_CHECK_STATE_HEADER:
        {
            rc = parse_headers(text, httpConnContextPtr);
            if (rc == HTTP_PARSTER_CODE_BAD_REQUEST) {
                httpConnContextPtr->setCode(HTTP_RC_CODE_BAD_REQUEST);
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
            httpConnContextPtr->setCode(HTTP_RC_CODE_INTERNAL_ERROR);
            return HTTP_PARSTER_CODE_INTERNAL_ERROR;
        }
    }
    return TCP_BUFFER_EVENT_TYPE_RECV;
}

int HTTPDConnHandler::add_response(HTTPConnContextPtr contextPtr, const char *format, ...)
{
    if (contextPtr->getWriteIdx() >= contextPtr->getWritebufferSize()) {
        return TCP_EVENT_TYPE_SEND_BUFFER_FULL;
    }

    va_list arg_list;
    va_start(arg_list, format);
    int len = vsnprintf(contextPtr->getSendBuffer() + contextPtr->getWriteIdx(),
                TCP_WRITE_BUFFER_SIZE - 1 - contextPtr->getWriteIdx(),
                format, arg_list);
    if (len >= (TCP_WRITE_BUFFER_SIZE - 1 - contextPtr->getWriteIdx()))
    {
        va_end(arg_list);
        return TCP_EVENT_TYPE_SEND_BUFFER_NOMEM;
    }
    contextPtr->setWriteIdx(contextPtr->getWriteIdx() + len);
    va_end(arg_list);

    return 0;
}

bool HTTPDConnHandler::add_status_line(HTTPDConnContextPtr httpConnContextPtr, int status, const char *title)
{
    return add_response(httpConnContextPtr, "%s %d %s\r\n", "HTTP/1.1", status, title);
}

bool HTTPDConnHandler::add_content_length(HTTPDConnContextPtr httpConnContextPtr, int content_len)
{
    return add_response(httpConnContextPtr, "Content-Length:%d\r\n", content_len);
}

bool HTTPDConnHandler::add_blank_line(HTTPDConnContextPtr httpConnContextPtr)
{
    return add_response(httpConnContextPtr, "%s", "\r\n");
}

bool HTTPDConnHandler::add_linger(HTTPDConnContextPtr httpConnContextPtr)
{
    return add_response(httpConnContextPtr, "Connection:%s\r\n", (
                    httpConnContextPtr->getlinger() == true) ? 
                                    "keep-alive" : "close");
}

bool HTTPDConnHandler::add_headers(HTTPDConnContextPtr httpConnContextPtr, int content_len)
{
    return add_content_length(httpConnContextPtr, content_len) && add_linger(httpConnContextPtr) &&
           add_blank_line(httpConnContextPtr);
}

bool HTTPDConnHandler::add_content(HTTPDConnContextPtr httpConnContextPtr, const char *content)
{
    return add_response(httpConnContextPtr, "%s", content);
}

int HTTPDConnHandler::handle_response(HTTPDConnContextPtr contextPtr)
{
    int rc = 0;

    HTTPDConnContextPtr contextPtr = std::dynamic_pointer_case<HTTPDConnContext>(contextPtr);
    switch (contextPtr->getRecode()) {
        case HTTP_RC_CODE_INTERNAL_ERROR: {
            add_status_line(contextPtr, HTTP_RC_CODE_INTERNAL_ERROR, error_500_title);
            add_headers(contextPtr, strlen(error_500_form));
            rc = add_content(contextPtr, error_500_form);
            if (rc != 0) {
                return rc;
            }
            break;
        } case HTTP_RC_CODE_BAD_REQUEST: {
            add_status_line(contextPtr, HTTP_RC_CODE_BAD_REQUEST, error_404_title);
            add_headers(contextPtr, strlen(error_404_form));
            rc = add_content(contextPtr, error_404_form)
            if (rc != 0) {
                return rc;
            }
            break;
        } case HTTP_RC_CODE_FORBIDDEN_REQUEST: {
            add_status_line(contextPtr, HTTP_RC_CODE_FORBIDDEN_REQUEST, error_403_title);
            add_headers(contextPtr, strlen(error_403_form));
            rc = add_content(contextPtr, error_403_form)
            if (rc != 0) {
                return rc;
            }
            break;
        } case HTTP_RC_CODE_OK: {
            add_status_line(contextPtr, HTTP_RC_CODE_OK, ok_200_title);
            const char *ok_string = httpConnContextPtr->m_resp.getBody().c_str();
            //const char *ok_string = "<html><body></body></html>";
            add_headers(contextPtr, strlen(ok_string));
            rc = add_content(contextPtr, ok_string);
            if (rc != 0) {
                return rc;
            }
            break;
        }
    }

    struct iovec send_iovec[2];
#if 0
    send_iovec[0].iov_base = httpConnContextPtr->getWritebuffer();
    send_iovec[0].iov_len = httpConnContextPtr->getWriteIdx();
    send_iovec[1].iov_base = httpConnContextPtr->m_resp.getBody();
    send_iovec[1].iov_len = httpConnContextPtr->m_resp.getBodyLenght();
    httpConnContextPtr->setSendIOV(&send_iovec, 2);
    httpConnContextPtr->setSendRestSize(httpConnContextPtr->getWriteIdx(),
                    httpConnContextPtr->m_resp.getBodyLenght());
#endif
    httpConnContextPtr->setSendIOV(&send_iovec, 1);
    httpConnContextPtr->setSendRestSize(httpConnContextPtr->getWriteIdx())
    return 0;
}


std::shared_ptr<HTTPDConnHandler> g_httpdhanderPtr;


__attribute__((constructor)) static void hander_init(void)
{
    g_httpdhanderPtr = std::shared_ptr<HTTPDConnHandler>();
    std::shared_ptr<TcpConnHandler> handerPtr = std::dynamic_pointer_cast<TcpConnHandler>(g_httpdhanderPtr);
    TcpConnManager::register_handler(handerPtr);
}


__attribute__((disstructor)) static void hander_exit(void)
{
    std::shared_ptr<TcpConnHandler> handerPtr = std::dynamic_pointer_cast<TcpConnHandler>(g_httpdhanderPtr);
    TcpConnManager.unregister_handler();
}


