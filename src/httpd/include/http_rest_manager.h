#ifndef __HTTPD_ROUTER_MANAGER_H__
#define __HTTPD_ROUTER_MANAGER_H__


#include <memory>
#include <list>
#include "http_service_manager.h"
#include "http_router.h"
#include "http_handler.h"


using namespace std;


class HTTPRestManager
{
public:
    HTTPRestManager() {}
    virtual ~HTTPRestManager() {}
    int init() { return 0; }
    int exit() { return 0; }
private:
    int registerDispather(HTTPRouterPtr &routerPtr, HTTPHandlerPtr &handlePtr) { return 0; }
    int unregisterDispather(HTTPRouterPtr &routerPtr) { return 0; }

protected:
    HTTPServiceManager m_httpServiceManager;
    std::list<HTTPRouterPtr> m_routerQ;
};


typedef std::shared_ptr<HTTPRestManager> HTTPRestManagerPtr;


#endif
