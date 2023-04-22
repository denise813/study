#ifndef __HTTPD_HANDLER_H__
#define __HTTPD_HANDLER_H__


#include <memory>


using namespace std;


class HTTPHandler
{
public:
    HTTPHandler() {}
    virtual ~HTTPHandler() {}
public:
    virtual int get() { return 0; }
    virtual int put()  { return 0; }
    virtual int post()  { return 0; }
    virtual int rm()  { return 0; }
    virtual int list()  { return 0; }
};


typedef std::shared_ptr<HTTPHandler> HTTPHandlerPtr;


#endif

