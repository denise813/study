#ifndef __HTTPD_CONTROLLER_H__
#define __HTTPD_CONTROLLER_H__


#include <memory>
#include "../include/http_router.h"
#include "../include/http_handler.h"


using namespace std;


class HTTPController : public HTTPRouter, public HTTPHandler
{
public:
   HTTPController();
    virtual ~HTTPController();
};


typedef std::shared_ptr<HTTPController> HTTPControllerPtr;


#endif
