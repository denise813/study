#include <stdio.h>
#include <stdlib.h>

//#include "httpd_log.h"
#include "src/httpd_config.h"
#include "src/httpd_server.h"


int main(int argc, char *argv[])
{
    int rc = 0;
    HTTPDConfigurePtr httpdConfigPtr = std::make_shared<HTTPDConfigure>();
    rc = httpdConfigPtr->init(argc, argv);
    if (rc < 0) {
        goto l_out;
    }

     HTTPDServerPtr httpServerPtr;
     httpServerPtr = std::make_shared<HTTPDServer>(httpdConfigPtr.get());
     rc= httpServerPtr->init();
    if (rc < 0) {
        goto l_exit_config;
    }
    rc = httpServerPtr->start();
    if (rc < 0) {
        goto l_exit_server;
    }

    httpServerPtr->stop();
    httpdConfigPtr->exit();
    rc = 0;

l_out:
    return rc;

l_exit_server:
    httpServerPtr->exit();

l_exit_config:
    httpdConfigPtr->exit();
    goto l_out;
}

