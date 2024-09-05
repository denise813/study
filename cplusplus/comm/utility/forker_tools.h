#ifndef _FORKER_TOOLS_H
#define _FORKER_TOOLS_H


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
    static virtual int damonize(int argc, char* argv[]);
    std::string get_image_path();
public:
    int execute(const std::string &cmd, cmd_result_t &out);
    int execute(const std::string &cmd,
                    int (*callback)(char * buff, int buffSize, cmd_result_t &out),
                    cmd_result_t &out);
private:
    int execute_exec(const std::string &cmd, cmd_result_t &out);
    int execute_popen(const std::string &cmd, cmd_result_t &out);
    int execute_popen(const std::string &cmd,
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
    bool is_owner();
    int init();
    int exit();
private:
    int try_lock();
    int unlock();
private:
    std::mutex m_mutex;
    std::string m_lockfile;
    int m_lockstatus;
    int m_fd = -1;
};



#endif
