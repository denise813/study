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


class HTTPDController
{
public:
    HTTPDController() = default;
    ~HTTPDController() = default;
public:
    virtual int process(http_req_t * req,  http_resp_t * resp) = default;
};


typedef std::shared_ptr<HTTPDController> HTTPDControllerPtr;


#endif
