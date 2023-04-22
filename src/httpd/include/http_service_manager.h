#ifndef __HTTPD_SERVICE_MANAGER_H__
#define __HTTPD_SERVICE_MANAGER_H__


#include <memory>
#include "cpp-httplib/httplib.h"


using namespace std;


class HTTPServiceManager
{
public:
    HTTPServiceManager();
    virtual ~HTTPServiceManager();
    int init();
    int exit();
    int run();
 private:
    int recvEntry();
    httplib::Server m_server;
};


typedef std::shared_ptr<HTTPServiceManager> HTTPServiceManagerPtr;


#endif
