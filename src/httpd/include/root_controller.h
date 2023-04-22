#ifndef __HTTPD_ROOT_CONTROLLER_H__
#define __HTTPD_ROOT_CONTROLLER_H__


#include <memory>
#include "../include/controller.h"


using namespace std;


class RootController : public HTTPController
{
public:
   RootController();
    virtual ~RootController();
};


typedef std::shared_ptr<RootController> RootControllerPtr;


#endif
