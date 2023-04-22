#ifndef _YS_FORKER_H
#define _YS_FORKER_H


#include <string>


using namespace std;



struct cmd_result_t
{
    int code;
    std::string out;
    std::string error;
};


struct Forker
{
    Forker(std::string moduleName);
    virtual ~Forker();
    virtual int damonize(int argc, char* argv[]);
    int execute(std::string &cmd, cmd_result_t &out);
    int execute(std::string &cmd, cmd_result_t &out,
                    int (*callback)(char * buff, int buffSize));
    int getCurrentExecDir(std::string &ExecDir);
    int setLDPath(std::string &path);
    void * loadLD(std::string soName);
    int unloadLD(void * ldInst);
// data
std::string m_modlueName;
};




#endif
