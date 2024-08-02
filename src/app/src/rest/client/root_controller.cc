#include "../include/root_controller.h"


RootController::RootController()
{
}

RootController::~RootController()
{
}


int RootController::dispather(
                std::string uri,
                std::string method,
                std::string params,
                CJosnPtr reqBodyPtr,
                CJsonPtr rspBodyPtr)
{
    int index = find(uri, "/");
    if (index >= 0) {
        
    }
    std::string rest = "";
    if (rest.size() != 0){
        m_restPtr->dispather();
    }
    return 0;
}

int RootController::handle_get()
{
    return 0;
}

int RootController::handle_put()
{
    return 0;
}

int RootController::handle_post()
{
    return 0;
}

int RootController::handle_delete()
{
    return 0;
}

