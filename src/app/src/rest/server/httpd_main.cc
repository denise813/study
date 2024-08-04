#include <stdio.h>
#include <stdlib.h>

//#include "httpd_log.h"
#include "rest/src/httpd_manager.h"


int main(int argc, char *argv[])
{
    int rc = 0;
    HTTPServerConfigPtr httpd_config_ptr = std::make_shared<HTTPServerConfig>();
    HTTPServerManagerPtr httpd_manager_ptr;
    RootControllerPtr root_controller_ptr;

    rc = httpd_config_ptr->init(argc, argv);
    if (rc < 0) {
        goto l_out;
    }
    httpd_manager_ptr = std::make_shared<HTTPServerManager>(httpd_config_ptr.get());
    root_controller_ptr = std::make_shared<RootController>(httpd_config_ptr.get());
    rc = httpd_manager_ptr->init(root_controller_ptr);
    if (rc < 0) {
        goto l_exit_config;
    }
    rc = httpd_manager_ptr->run();
    if (rc < 0) {
        goto l_exit_manager;
    }

    httpd_manager_ptr->stop();
    httpd_manager_ptr->exit();
    httpd_config_ptr->exit();
    rc = 0;

l_out:
    return rc;

l_exit_manager:
    httpd_manager_ptr->exit();

l_exit_config:
    httpd_config_ptr->exit();
    goto l_out;
}

