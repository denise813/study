#include "httpd_controller.h"


static std::vector< std::sharedPtr<HTTPDController> > g_subControllers;


HTTPDController::HTTPDController()
{
}


HTTPDController::~HTTPDController()
{
}

int HTTPDController::dispather(HTTPDConnContextPtr contextPtr)
{
    for(auto itor : g_subControllers) {
        itor->dispather();
    }
}

HTTPDController::register(std::shared_ptr<HTTPDController> controllerPtr)
{
    g_subControllers.push_back(controllerPtr);
}


