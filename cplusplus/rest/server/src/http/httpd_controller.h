#ifndef HTTPD_HANDLER_CONTROLLER_H
#define HTTPD_HANDLER_CONTROLLER_H


#include <cassert>
#include <memory>



class HTTPDController
{
public:
    HTTPDController() = default;
    ~HTTPDController() = default;
public:
    static register(std::shared_ptr<HTTPDController> controllerPtr);
public:
    virtual int process(HTTPDConnContextPtr contextPtr);
    virtual int dispather(HTTPDConnContextPtr contextPtr);

};


typedef std::shared_ptr<HTTPDController> HTTPDControllerPtr;


#endif
