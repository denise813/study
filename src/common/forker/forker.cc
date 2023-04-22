#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <dlfcn.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string>
#include <sstream>
#include <memory>
#include "forker.h"


using namespace std;


#define POPEN_BUFF_MAX_LEN (4096)

Forker::Forker(std::string moduleName)
{
    m_modlueName = moduleName;
}

Forker::~Forker()
{
}

int Forker::damonize(int argc, char* argv[])
{
/* --comment by louting, 2023/3/16--
 * nodir and close STDIN, STDOUT, STDERR
 */
    int rc = 0;
    rc = daemon(1, 0);
    if (rc < 0) {
        rc = -errno;
        return rc;
    }
    return 0;
}

int Forker::execute(std::string &cmd, cmd_result_t &out)
{
    int rc = 0;
    stringstream py_stdout;
    stringstream * recv_stream;
    recv_stream = &py_stdout;
    std::string real_cmd  = cmd + " 2>&1";
    FILE * popenStream = popen(real_cmd.c_str(), "r");
    if (popenStream == nullptr) {
        rc = -errno;
        return rc;
    }

    char tmp[POPEN_BUFF_MAX_LEN] = {0};
    while (!feof(popenStream)) {
        memset(tmp, 0, sizeof(tmp));
        rc = fread(tmp, 1, POPEN_BUFF_MAX_LEN -1, popenStream);
        if(rc < 0) {
            rc = -errno;
            pclose(popenStream);
            return rc;
        }
        (*recv_stream )<< tmp;
    }
    out.out = py_stdout.str();
    out.code = pclose(popenStream);
    return out.code;
}

int Forker::execute(std::string &cmd, cmd_result_t &out, int (*callback)(char * buff, int buffSize))
{
    int rc = 0;
    std::string real_cmd  = cmd + " 2>&1";
    FILE * popenStream = popen(real_cmd.c_str(), "r");
    if (popenStream == nullptr) {
        rc = -errno;
        return rc;
    }

    char tmp[POPEN_BUFF_MAX_LEN] = {0};
    while (!feof(popenStream)) {
        memset(tmp, 0, sizeof(tmp));
        rc = fread(tmp, 1, POPEN_BUFF_MAX_LEN -1, popenStream);
        if(rc < 0) {
            rc = -errno;
            pclose(popenStream);
            return rc;
        }
        rc = callback(tmp, rc);
        if (rc < 0) {
            pclose(popenStream);
            return rc;
        }
    }
    rc = pclose(popenStream);
    return rc;
}

int Forker::getCurrentExecDir(std::string &ExecDir)
{
    char pidInfo[1023] = {0};
    snprintf(pidInfo, 1023, "/proc/%d/exe", getpid());

    char exeFullPath[1023 + 1] = {0};
    readlink(pidInfo, exeFullPath, 1000);
    std::string imagePath = exeFullPath;
    std::string imageDir= imagePath.substr(0, imagePath.rfind("/"));
    ExecDir = imageDir;
    return 0;
}

int Forker::setLDPath(std::string &path)
{
    unsetenv("LD_LIBRARY_PATH");
    char ldlinkLibpath[1023] = {0};
    snprintf(ldlinkLibpath, 1023, "LD_LIBRARY_PATH=%s", path.c_str());
    putenv(ldlinkLibpath);
    return 0;
}

void * Forker::loadLD(std::string soName)
{
    void* ldInst = dlopen(soName.c_str(), RTLD_LAZY);
    return ldInst;
}

int Forker::unloadLD(void * ldInst)
{
    if(ldInst) {
        dlclose(ldInst);
    }
    return 0;
}

