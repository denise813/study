
#include "../include/http_service_manager.h"


HTTPServiceManager::HTTPServiceManager()
{
}

HTTPServiceManager::~HTTPServiceManager()
{
}


int HTTPServiceManager::init()
{
     m_server.listen("0.0.0.0", 8081);
    return 0;
}

int HTTPServiceManager::exit()
{
    return 0;
}

int HTTPServiceManager::recvEntry()
{
    return 0;
}

int HTTPServiceManager::run()
{
    return 0;
}

