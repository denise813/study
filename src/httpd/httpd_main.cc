#include <stdio.h>
#include <stdlib.h>
#include "cpp-httplib/httplib.h"
#include "http_rest_manager.h"
#include "http_service_manager.h"


void doGetHi(const httplib::Request& req, httplib::Response& res)
{
    res.set_content("hi", "text/plain");
}

void doGetHi_HAHA(const httplib::Request& req, httplib::Response& res)
{
    res.set_content("haha", "text/plain");
}


int main(int argc, char *argv[])
{
    httplib::Server server;
#if 0
    HTTPServiceManagerPtr httpServiceManagerPtr = std::make_shared<HTTPServiceManager>();
    HTTPRestManagerPtr httpRestManagerPtr = std::make_shared<HTTPRestManager>(httpServiceManagerPtr);
    httpRestManagerPtr->init();
    httpServiceManagerPtr->init();
    httpServiceManagerPtr->run();
    httpServiceManagerPtr->exit();
    httpRestManagerPtr->exit();
#endif
    server.Get("/hi", doGetHi);
    server.Get("/hi/haha", doGetHi);

    server.listen("0.0.0.0", 8081);
    return 0;
}

