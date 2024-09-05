#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/file.h>


#include <string>
#include <sstream>
#include <memory>

#include "logger.h"
#include "string_format_tools.hpp"
#include "forker_tools.h"


using namespace std;


#define COM_TRACE_LOG_IDS_TRACE_BUFF 10
#define POPEN_BUFF_MAX_LEN (4096)


SubProcess::SubProcess(std::string cmd)
{
    m_cmd = cmd;
}

SubProcess::~SubProcess()
{
}

int SubProcess::pipeOpen(int pipefd[2])
{
    int  rc = 0;
    if (pipe(pipefd) == -1) {
        rc = -errno;
        return rc;
    }
    rc = fcntl(pipefd[0], F_SETFD, FD_CLOEXEC);
    if (rc < 0) {
        pipeClose(pipefd);
        rc = -errno;
        return rc;
    }

    rc = fcntl(pipefd[1], F_SETFD, FD_CLOEXEC);
    if (rc < 0) {
        pipeClose(pipefd);
        rc = -errno;
        return rc;
    }

    return 0;
}

int SubProcess::pipeClose(int pipefd[2])
{
    close(pipefd[0]);
    close(pipefd[1]);
    return 0;
}

int SubProcess::pipeSpawn()
{
    int rc = 0;
    int pipe[2] = {-1, -1};

    rc = pipeOpen(pipe);
    if (rc < 0) {
        return rc;
    }

    int pid = -1;
    pid = fork();
    if (pid < 0) {
        rc = -errno;
        pipeClose(pipe);
        return rc;
    }

    if (pid == 0) {
        // child
        close(STDOUT_FILENO);
        dup(pipe[1]);
        execl("/bin/sh","-c",m_cmd.c_str(), NULL);
        //exit(0);
    }

    int status = -1;
    int childpid = wait(&status);
    if (childpid < 0) {
        return -errno;
    }
    printf("childpid=%d\n", childpid);
    // Parent
    close(pipe[1]);
    m_pid = pid;
    m_file =fdopen(pipe[0],"r");
    if (m_file == nullptr) {
        rc = -errno;
        return rc;
    }
    return 0;
}

int SubProcess::pipeRead(std::string &out)
{
    int rc = 0;
    stringstream py_stdout;
    out.clear();
    char tmp[POPEN_BUFF_MAX_LEN] = {0};
    while (!feof(m_file)) {
        memset(tmp, 0, sizeof(tmp));
        rc = fread(tmp, 1, POPEN_BUFF_MAX_LEN -1, m_file);
        if(rc < 0) {
            rc = -errno;
            return rc;
        }
        py_stdout << tmp;
    }
    out = py_stdout.str();

    return 0;
}

int SubProcess::pipeJoin()
{
    int rc = 0;
    int status;

    while(1) {
        rc = waitpid(m_pid, &status, 0);
        if (rc < 0) {
            break;
        }
    }

    m_pid = -1;
    if (WIFEXITED(status)) {
        fclose(m_file);
        m_file = nullptr;
        return WEXITSTATUS(status);
    }
    if (WIFSIGNALED(status)) {
        fclose(m_file);
        m_file = nullptr;
        return WTERMSIG(status);
    }

    fclose(m_file);
    m_file = nullptr;
    return EXIT_FAILURE;
}

int SubProcess::spawn()
{
    return pipeSpawn();
}

int SubProcess::join()
{
    return pipeJoin();
}

int SubProcess::getout(std::string &out)
{
    return pipeRead(out);
}


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

#if 0
int Forker::XShowArgv(int index, char* argv[], char * sub)
{
    argv[index] = sub;
    return 0;
}
#endif

int Forker::execute(const std::string &cmd, cmd_result_t &out)
{
    return executePopen(cmd, out);
}

string Forker::get_image_path()
{
	char pidInfo[PATH_MAX] = {0};
	snprintf(pidInfo, PATH_MAX, "/proc/%d/exe", getpid());

	char exeFullPath[PATH_MAX + 1] = {0};
	readlink(pidInfo, exeFullPath, PATH_MAX);

	string imagePath = exeFullPath;

	return imagePath.substr(0, imagePath.rfind("/"));
}


int Forker::execute(const std::string &cmd,
                int (*callback)(char * buff, int buffSize, cmd_result_t &out),
                cmd_result_t &out)
{
    return executePopen(cmd, callback, out);
}


int Forker::executeExec(const std::string &cmd, cmd_result_t &out)
{
    int rc = 0;
    SubProcess executer(cmd);
    rc = executer.spawn();
    if (rc < 0) {
        out.code = rc;
        return rc;
    }

    rc = executer.getout(out.out);
    if (rc < 0) {
        out.code = rc;
        return rc;
    }

    rc = executer.join();
    if (rc < 0) {
        out.code = rc;
        return rc;
    }

    out.code = rc;
    
    return rc;
}

int Forker::execute_popen(const std::string& cmd, cmd_result_t& out)
{
    int rc = 0;
    stringstream py_stdout;
    stringstream * recv_stream;
    out.code = 0;
    out.error = "";
    out.out = "";
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
            out.code = rc;
            pclose(popenStream);
            return rc;
        }
        (*recv_stream )<< tmp;
    }
    out.out = py_stdout.str();
#if 0
    uint64_t outlen = py_stdout.str().length();
    VLOGDEBUG(COM_TRACE_LOG_IDS_TRACE_BUFF, m_modlueName,
                    "len=%lu, out(%s)",
                    outlen, out.out.c_str());
#endif
    rc = pclose(popenStream);
    if (rc == -1) {
        out.code = -errno;
    } else {
        out.code = rc;
        if (WIFEXITED(rc)) {
            out.code = WEXITSTATUS(rc);
        } else if (WIFSIGNALED(rc)) {
            out.code = WTERMSIG(rc);
        }
    }
    return out.code;
}

int Forker::execute_popen(const std::string &cmd,
                int (*callback)(char * buff, int buffSize, cmd_result_t &out),
                cmd_result_t &out)
{
    int rc = 0;
    std::string real_cmd  = cmd + " 2>&1";
    FILE * popenStream = popen(real_cmd.c_str(), "r");
    if (popenStream == nullptr) {
        rc = -errno;
        out.code = rc;
        return rc;
    }

    char tmp[POPEN_BUFF_MAX_LEN] = {0};
    while (!feof(popenStream)) {
        memset(tmp, 0, sizeof(tmp));
        rc = fread(tmp, 1, POPEN_BUFF_MAX_LEN -1, popenStream);
        if(rc < 0) {
            rc = -errno;
            out.code = rc;
            pclose(popenStream);
            return rc;
        }
        rc = callback(tmp, rc, out);
        if (rc < 0) {
            pclose(popenStream);
            return rc;
        }
    }
#if 0
    uint64_t outlen = py_stdout.str().length();
    VLOGDEBUG(COM_TRACE_LOG_IDS_TRACE_BUFF, m_modlueName,
                    "len=%lu, out(%s)",
                    outlen, out.out.c_str());
#endif
    rc = pclose(popenStream);
    if (rc == -1) {
        out.code = -errno;
    } else {
        out.code = rc;
        if (WIFEXITED(rc)) {
            out.code = WEXITSTATUS(rc);
        } else if (WIFSIGNALED(rc)) {
            out.code = WTERMSIG(rc);
        }
    }

    return out.code;
}


int Forker::setEnv(const std::string &name, const std::string & value)
{
    int rc = 0;
    rc = unsetenv(name.c_str());
    if (rc < 0) {
        rc = -errno;
        return rc;
    }
    rc = setenv(name.c_str(), value.c_str(), 1);
     if (rc < 0) {
        rc = -errno;
        return rc;
    }
    return 0;
}

int Forker::setLDLinkPath(const std::string &path)
{
    return setEnv("LD_LIBRARY_PATH", path);
}


int Forker::bugOn()
{
    int a = 1;
    int b = 0;
     VLOGFATAL(COM_TRACE_LOG_IDS_TRACE_BUFF, m_modlueName,
                    "failed exception!");
    int bug = a / b;
    
    return bug;
}


enum process_locker_status_e
{
    PROCESSLOCKER_STATUS_LOCK,
    PROCESSLOCKER_STATUS_UNLOCK,
    PROCESSLOCKER_STATUS_OTHERLOCK,
};

ProcessLocker::ProcessLocker(std::string lockfile)
{
    m_lockfile = lockfile;
    m_lockstatus = PROCESSLOCKER_STATUS_UNLOCK;
}

ProcessLocker::~ProcessLocker()
{
    unlock();
}

int ProcessLocker::init()
{
    int rc = 0;

    rc = try_lock();
    if (rc < 0) {
        return rc;
    }

    return 0;
}

int ProcessLocker::exit()
{
    return 0;
}

int ProcessLocker::try_lock()
{
    int rc = 0;
    int fd = 0;
    std::unique_lock <std::mutex> waitLock(m_mutex);
    fd = open(m_lockfile.c_str(), O_RDWR|O_CREAT, 0666);
    if (fd < 0) {
        rc = -errno;
        return rc;
    }
    m_fd = fd;
    rc = flock(m_fd, LOCK_EX|LOCK_NB);
    if (rc < 0) {
        rc = -errno;
        if (rc == -EWOULDBLOCK) {
            m_lockstatus = PROCESSLOCKER_STATUS_OTHERLOCK;
            rc = 0;
        }
        return rc;
    }
    m_lockstatus = PROCESSLOCKER_STATUS_LOCK;
    return 0;
}

int ProcessLocker::unlock()
{
    if (m_fd < 0) { return 0; }

    close(m_fd);
    m_lockstatus = PROCESSLOCKER_STATUS_UNLOCK;
    return 0;
}

bool ProcessLocker::is_owner()
{
    if (m_lockstatus == PROCESSLOCKER_STATUS_OTHERLOCK) {
        return false;
    }
    return true;
}

#if 0
int call_program(
                const char * process_name,
                const char *cmd,
                callback_t callback,
                void *data,
                char *output,
                int op_len,
                int flags)
{
    pid_t pid;
    int fds[2] = {-1, -1};
    int rc = 0;
    int i = 0;
    char *pos, arg[256];
    char *argv[sizeof(arg) / 2];

    i = 0;
    pos = arg;
    str_spacecpy(&pos, cmd);
    if (strchr(cmd, ' ')) {
        while (*pos != '\0')
            argv[i++] = strsep(&pos, " ");
    } else
        argv[i++] = arg;
    argv[i] =  NULL;

    rc = pipe(fds);
    if (rc < 0) {
        rc = -errno;
       return rc;
    }

    pid = fork();
    if (pid < 0) {
        close(fds[0]);
        close(fds[1]);
        return pid;
    }

    if (!pid) {
        close(1);
        rc = dup(fds[1]);
        if (rc < 0) {
            rc = errno;
            exit(rc);
        }
        close(fds[0]);
        execv(argv[0], argv);
        rc = errno;
        exit(rc);
    } else {
        struct timeval tv;
        fd_set rfds;
        int ret_sel;

        close(fds[1]);
        prctl(PR_SET_NAME, process_name, NULL, NULL, NULL);
        do {
            FD_ZERO(&rfds);
            FD_SET(fds[0], &rfds);
            tv.tv_sec = 0;
            tv.tv_usec = 100000;
            ret_sel = select(fds[0]+1, &rfds, NULL, NULL, &tv);
        } while (ret_sel < 0 && errno == EINTR);

        if (ret_sel <= 0) { /* error or timeout */
            kill(pid, SIGTERM);
        }
        do {
            rc = waitpid(pid, &i, 0);
        } while (ret < 0 && errno == EINTR);
        if (rc < 0) {
            close(fds[0]);
            return rc;
        }
        if (ret_sel > 0) {
            rc = read(fds[0], output, op_len);
            if (rc < 0) {
                rc = -errno;
                close(fds[0]);
                return rc;
            }
        }

        if (callback) {
            callback(data, WEXITSTATUS(i));
        }
        close(fds[0]);
    }

    return 0;
}
#endif

