#ifndef _UTILS_FORKER_H
#define _UTILS_FORKER_H


#include <string>
#include <mutex>


using namespace std;


struct cmd_result_t
{
    int code;
    std::string out;
    std::string error;
};


class SubProcess
{
public:
    SubProcess(std::string cmd);
    ~SubProcess();
     int spawn();
     int join();
     int getout(std::string &out);
private:
    int pipeOpen(int pipefd[2]);
    int pipeClose(int pipefd[2]);
    int pipeSpawn();
    int pipeRead(std::string &out);
    int pipeJoin();
private:
    std::string m_cmd;
    pid_t m_pid = -1;
    FILE* m_file = nullptr;
};


class Forker
{
public:
    Forker(std::string moduleName);
    virtual ~Forker();
    virtual int damonize(int argc, char* argv[]);
    int execute(const std::string &cmd, cmd_result_t &out);
    int execute(const std::string &cmd,
                    int (*callback)(char * buff, int buffSize, cmd_result_t &out),
                    cmd_result_t &out);
    int setEnv(const std::string &name, const std::string &value);
    int setLDLinkPath(const std::string &path);
    int bugOn();
private:
    int executeExec(const std::string &cmd, cmd_result_t &out);
    int executePopen(const std::string &cmd, cmd_result_t &out);
    int executePopen(const std::string &cmd,
                    int (*callback)(char * buff, int buffSize, cmd_result_t &out),
                    cmd_result_t &out);
#if 0
    int XShowArgv(int index, char* argv[], char * sub="xxxxxx");
#endif
// data
private:
    std::string m_modlueName;
};


class ProcessLocker
{
public:
    ProcessLocker(std::string lockfile);
    ~ProcessLocker();
public:
    bool isOwner();
    int build();
private:
    int trylock();
    int unlock();
private:
    std::mutex m_mutex;
    std::string m_lockfile;
    int m_lockstatus;
    int m_fd = -1;
};



#endif
