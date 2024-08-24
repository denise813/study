#include "httpd_controller.h"


HTTPDRootController::HTTPDRootController()
{
}


HTTPDRootController::~HTTPDRootController()
{
}

int HTTPDRootController::dispather(HTTPDConnContextPtr contextPtr)
{
    
}


__attribute__((constructor)) static void iscsi_transport_init(void)
{
    std::shared_ptr<HTTPDRootController> controllerPtr = std::shared_ptr<HTTPDRootController>();
    HTTPDController::register(controllerPtr);
}

