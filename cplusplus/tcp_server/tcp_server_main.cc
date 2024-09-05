#include <string.h>
#include <unistd.h>

#include <memory>

#include "src/config.h"
#include "src/mgr.h"
#include "src/log.h"
#include "src/server.h"


//#include "work.h"
//#include "util.h"


using namespace std;


const char * get_current_processName(const char * cmd_path)
{
    const char * cmd = nullptr;
    if ((cmd = strrchr(cmd_path, '/')) == NULL){
        cmd = cmd_path;
    } else {
        cmd++;
    }
    return cmd;
}


int main(int argc, char **argv)
{

    int rc = 0;

    TcpConfigPtr configPtr = std::make_shared<TcpConfig>();
    TcpLoggerMgrPtr loggerPtr = std::make_shared<TcpLoggerMgr>();
    TcpServerPtr serverPtr = std::make_shared<TcpServer>();
    std::string program_name;


    program_name = std::string(get_current_processName(argv[0]));
    rc = configPtr->init();
    if (rc < 0) {
        return rc;
    }
    if (configPtr->is_deamon()) {
        rc = daemon(0, 1);
        if(rc < 0) {
            rc = -errno;
        }
        return rc;
    }

    rc = loggerPtr->init(program_name);
    if (rc < 0) {
        exit(1);
    }

    rc = serverPtr->init(configPtr.get());
    if (rc < 0) {
        return rc;
    }

    rc = serverPtr->start();
    if (rc < 0) {
        return rc;
    }

    (void)serverPtr->loop();

    serverPtr->exit();

#if 0
    loggerPtr->log_close();
#endif
    configPtr->exit();

    return 0;
}
