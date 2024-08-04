#include "../include/controller.h"


HTTPController::HTTPController()
{
}

HTTPController::~HTTPController()
{
}

int HTTPController::dispather(std::string uri,
                std::string method,
                std::string params,
                CJosnPtr reqBodyPtr,
                CJsonPtr rspBodyPtr)
{
    int index = find(uri, m_uri);
    if (index >= 0) {
        
    }
    std::string rest = "";
    if (rest.size() != 0){
        m_restPtr->dispather();
    } else {
        if (method == "POST") {
            return handle_post(uri, params, reqBodyPtr, rspBodyPtr);
        } else if (method == "GET") {
            return handle_get(uri, params, reqBodyPtr, rspBodyPtr);
        } else if (method == "PUT") {
            return handle_put(uri, params, reqBodyPtr, rspBodyPtr);
        } else if (method == "DELETE") {
            return handle_delete(uri, params, reqBodyPtr, rspBodyPtr);
        } 
    }

    return 0;
}

int HTTPController::handle_get()
{
    return 0;
}

int HTTPController::handle_put()
{
    return 0;
}

int HTTPController::handle_post()
{
    return 0;
}

int HTTPController::handle_delete()
{
    return 0;
}


