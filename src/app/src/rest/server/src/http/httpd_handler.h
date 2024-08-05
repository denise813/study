#ifndef HTTPD_HANDLER_H
#define HTTPD_HANDLER_H


#include <cassert>
#include <memory>

#include "httpd_config.h"
#include "tcp_conn_handler.h"


#define HTTPD_LINE_BUFFER_SIZE 2048


enum HTTP_METHOD
{
    HTTP_METHOD_GET = 0,
    HTTP_METHOD_POST,
    HTTP_METHOD_HEAD,
    HTTP_METHOD_PUT,
    HTTP_METHOD_DELETE,
    HTTP_METHOD_TRACE,
    HTTP_METHOD_OPTIONS,
    HTTP_METHOD_CONNECT,
    HTTP_METHOD_PATH
};


enum HTTP_CHECK_STATE
{
    HTTP_CHECK_STATE_REQUESTLINE = 0,
    HTTP_CHECK_STATE_HEADER,
    HTTP_CHECK_STATE_CONTENT
};


enum HTTP_PARSTER_CODE
{
    HTTP_PARSTER_CODE_NO_REQUEST,
    HTTP_PARSTER_CODE_GET_REQUEST,
    HTTP_PARSTER_CODE_BAD_REQUEST,
    HTTP_PARSTER_CODE_NO_RESOURCE,
    HTTP_PARSTER_CODE_FORBIDDEN_REQUEST,
    HTTP_PARSTER_CODE_OK,
    HTTP_PARSTER_CODE_FILE_REQUEST,
    HTTP_PARSTER_CODE_INTERNAL_ERROR,
    HTTP_PARSTER_CODE_CLOSED_CONNECTION
};

enum HTTP_RC_CODE
{
    HTTP_RC_CODE_OK = 200, //ok
    HTTP_RC_CODE_400 = 400,
    HTTP_RC_CODE_FORBIDDEN_REQUEST = 403,
    HTTP_RC_CODE_BAD_REQUEST = 404, //BAD_REQUEST
    HTTP_RC_CODE_INTERNAL_ERROR = 500 //INTERNAL_ERROR
};

enum HTTP_LINE_STATUS
{
    HTTP_LINE_STATUS_OK = 0,
    HTTP_LINE_STATUS_BAD,
    HTTP_LINE_STATUS_OPEN
};


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


class HTTPDConnHandler : public TcpConnHandler
{
public:
    HTTPDConnHandler(HTTPDConfigure* configPtr);
    ~HTTPDConnHandler();
public:
    virtual TcpConnContextPtr createContext(int fd);
    virtual int process_request(TcpConnContextPtr contextPtr);
    virtual int process_response(TcpConnContextPtr contextPtr);
    
private:
    HTTPDConfigure * m_configPtr;
    HTTPDController * m_controller;
};


typedef std::shared_ptr<HTTPDHandler> HTTPDHandlerPtr;


#endif
