#ifndef _YS_FORKER_H
#define _YS_FORKER_H


#include <string>


using namespace std;


namespace yuanshuo
{
namespace tools
{


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
// data
std::string m_modlueName;
};


};
};


#endif
