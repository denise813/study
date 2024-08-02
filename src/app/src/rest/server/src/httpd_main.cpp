#include "httpd_server.h"
#include "httpd_config.h"

int main(int argc, char *argv[])
{

    //命令行解析
    HTTPDConfigurePtr configPtr = std::make_shared<HTTPDConfigure>();
    configPtr->parse(argc, argv);

    HTTPDServerPtr serverPtr = std::make_shared<HTTPDServer>(configPtr.get());
    serverPtr->init();
    serverPtr->start();
    serverPtr->loop();
    serverPtr->stop();
    serverPtr->exit();

    return 0;
}