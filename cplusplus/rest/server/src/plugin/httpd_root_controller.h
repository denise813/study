#ifndef HTTPD_HANDLER_CONTROLLER_H
#define HTTPD_HANDLER_CONTROLLER_H


#include <cassert>
#include <memory>


typedef struct http_req
{
public:
    http_req() = default;
    ~http_req() = default;

    int setContentLenght(int len) { m_content_length = len; }
    int getContentLenght() { return m_content_length; }
    int getHost() { return m_host; }
    int setHost(char * host) { m_host = strdup(host); }
    int setString(char *txt) { m_string = strdup(txt); }
    void setMethod(int method) { m_method = method; }
    int getMethod() { return m_method; }

    char *m_url;
    char *m_version;
    char *m_host;
    HTTP_METHOD m_method;
    long m_content_length;
}http_req_t;


typedef struct http_resp
{
    http_resp() = default;
    ~http_resp() = default;

    void setRecode(int code) { m_recode = code; }
    int getRecode() { return m_recode; }

    int m_recode;
    long m_content_length;
}http_resp_t;


class HTTPDConnContext : public TcpConnContext
{
public:
    HTTPDConnContext(int fd): TcpConnContext(fd) {}
    ~HTTPDConnContext() = default;
public:
    int setCheckState(int check_state) { m_check_state = check_state; }
    int getCheckState() { return m_check_state; }
    int getStartLine() { return m_start_line; }
    void setStartLine(int start_line) { m_start_line = start_line; }
    void setlinger(bool linger) { m_linger = linger; }
    bool getlinger() { return m_linger; }
public:
    http_req_t m_req;
    http_resp_t m_resp;
    int int m_start_line;
    HTTP_CHECK_STATE m_check_state = HTTP_CHECK_STATE_REQUESTLINE;

    long m_content_length;
    bool m_linger;
    //char *m_file_address;

    //int cgi;        //是否启用的POST
    char *m_string; //存储请求头数据
};


typedef std::shared_ptr<HTTPDConnContext> HTTPDConnContextPtr;


class HTTPDController
{
public:
    HTTPDController() = default;
    ~HTTPDController() = default;
public:
    virtual int process(HTTPDConnContextPtr contextPtr);
    virtual int dispather(HTTPDConnContextPtr contextPtr);
};


typedef std::shared_ptr<HTTPDController> HTTPDControllerPtr;


#endif
