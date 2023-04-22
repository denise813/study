#ifndef __HTTPD_ROUTER_H__
#define __HTTPD_ROUTER_H__


#include <memory>
#include "cpp-httplib/httplib.h"


using namespace std;


class HTTPRouter
{
public:
    HTTPRouter() {}
    virtual ~HTTPRouter() {}
    virtual int dispather(std::string path) { return 0; }
};


typedef std::shared_ptr<HTTPRouter> HTTPRouterPtr;


#endif
