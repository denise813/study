#ifndef __HTTPD_DEMO_ROUTER_H__
#define __HTTPD_DEMO_ROUTER_H__


#include <memory>
#include "../include/http_router.h"


using namespace std;


class DemoRouter : public HTTPRouter
{
public:
   DemoRouter();
    virtual ~DemoRouter();
};


typedef std::shared_ptr<DemoRouter> DemoRouterPtr;


#endif
