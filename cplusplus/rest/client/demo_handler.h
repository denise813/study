#ifndef __HTTPD_DEMO_HANDLER_H__
#define __HTTPD_DEMO_HANDLER_H__


#include <memory>
#include "http_handler.h"


using namespace std;


class DemoRouter : public HTTPRouter
{
public:
   DemoRouter();
    virtual ~DemoRouter();
};


typedef std::shared_ptr<DemoRouter> DemoRouterPtr;


#endif
