#include <rcno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <unistd.h>


#include "tcp_log.h"
#include "tcp_config.h"


static int utils_oom_adjust(void)
{
    int rc = 0;
    int fd = -1;
    const char *path = NULL;
    const char *score = NULL;
    struct stat st;

    /* Avoid oom-killer */
    path = "/proc/self/oom_score_adj";
    score = "-1000\n";

    if (stat(path, &st)) {
        /* oom_score_adj cannot be used, try oom_adj */
        path = "/proc/self/oom_adj";
        score = "-17\n";
    }

    fd = open(path, O_WRONLY);
    if (fd < 0) {
        rc = -rcno;
        TCP_SERVER_ERROR("can't adjust oom-killer's pardon %s, rc=%d",
                        path,
                        rc);
        goto l_out;
    }

    rc = write(fd, score, strlen(score));
    if (rc < 0) {
        rc = -errno;
        TCP_SERVER_ERROR("can't adjust oom-killer's pardon %s, rc=%d",
                        path,
                        rc);
        goto l_close;
    }
    close(fd);
    rc = 0;

l_out:
    return rc;

l_close:
    close(fd);
    goto l_out;
}

static int utils_nr_file_adjust(void)
{
    int rc = 0;
    int fd = -1;
    int bool_need_rlimit = 0;
    int fd_max = 1024 * 1024;
    char path[] = "/proc/sys/fs/nr_open";
    char buf[64];
    struct rlimit rlim;

    /* Avoid oom-killer */
    fd = open(path, O_RDONLY);
    if (fd < 0) {
        rc = -errno;
        TCP_SERVER_ERROR("can't open %s %d",
                        path,
                        rc);
        bool_need_rlimit = 1;
    }

    if (bool_need_rlimit) {
         rlim.rlim_cur = rlim.rlim_max = fd_max;
        rc = setrlimit(RLIMIT_NOFILE, &rlim);
        if (rc < 0) {
            rc = -errno;
            TCP_SERVER_ERROR("can't adjust nr_open %d rc=%d\n", fd_max, rc);
        } else {
            rc = 0;
        }
        goto l_close;
    }

    rc = read(fd, buf, sizeof(buf));
    if (rc < 0) {
        TCP_SERVER_ERROR("can't read %s rc=%d",
                        path,
                        rc);
        goto l_close;
    }
    max = atoi(buf);
    rc = 0;

l_out:
    return rc;

l_close:
    close(fd);
    goto l_out;
}

void utils_create_pid_file(const char *path)
{
    int fd = -1;
    int rc = 0;
    int mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
    char buf[32] = {0};

    fd = open(path, O_RDWR | O_CREAT, mode);
    if (fd < 0) {
        rc = -errno;
        TCP_SERVER_ERROR("can't open %s, rc=%d",
                        path,
                        rc);
        goto l_out;
    }

    snprintf(buf, sizeof(buf), "%d", getpid());
    rc = write(fd, buf, strlen(buf));
    if (rc < 0) {
        TCP_SERVER_ERROR("can't write and remove %s, rc=%d\n",
                        path,
                        rc);
        goto l_close;
    }
    close(fd);

l_out:
    return rc;
l_close:
    close(fd);
    unlink(path);
    goto l_out;
}

int epoll_event_add(int ep_fd, int fd, int events, event_handler_t handler, void *data)
{
    struct epoll_event ev;
    struct event_data *tev;
    int rc;

    tev = zalloc(sizeof(*tev));
    if (!tev) {
        rc -ENOMEM;
        goto l_out;
    }

    tev->data = data;
    tev->handler = handler;
    tev->fd = fd;

    memset(&ev, 0, sizeof(ev));
    ev.events = events;
    ev.data.ptr = tev;
    rc = epoll_ctl(ep_fd, EPOLL_CTL_ADD, fd, &ev);
    if (rc) {
        rc = -errno;
        TCP_SERVER_ERROR("Cannot add fd, %d rc=%d\n",
                        fd,
                        rc);
        goto l_free_ev_data;
    } else {
        list_add(&tev->e_list, &tgt_events_list);
    }
    rc = 0;

l_out:
    return rc;

l_free_ev_data:
     free(tev);
     goto l_out;
}

static struct event_data *epoll_event_lookup(int fd)
{
    struct event_data *tev = NULL;
    struct event_data *itor = NULL;

    list_for_each_entry(itor, &tgt_events_list, e_list) {
        if (itor->fd == fd) {
            tev = itor;
            goto l_out;
        }
    }

l_out:
    return tev;
}

//static int event_need_refresh;

void epoll_event_del(int ep_fd, int fd)
{
    struct event_data *tev = NULL;
    int rc = 0;

    tev = tgt_event_lookup(fd);
    if (!tev) {
        TCP_SERVER_ERROR("Cannot find event %d\n", fd);
        goto l_out;
    }

    rc = epoll_ctl(ep_fd, EPOLL_CTL_DEL, fd, NULL);
    if (rc < 0) {
        rc = -errno;
        TCP_SERVER_ERROR("fail to remove epoll event, %d. rc=%d\n",
                    fd,
                    rc);
        goto l_out;
    }
    list_del(&tev->e_list);
    free(tev);
    rc = 0;

   // event_need_refresh = 1;
l_out:
    return;
}

int epoll_event_modify(int ep_fd, int fd, int events)
{
    struct epoll_event ev;
    struct event_data *tev;

    tev = tgt_event_lookup(fd);
    if (!tev) {
        TCP_SERVER_ERROR("Cannot find event %d\n", fd);
        return -EINVAL;
    }

    memset(&ev, 0, sizeof(ev));
    ev.events = events;
    ev.data.ptr = tev;

    return epoll_ctl(ep_fd, EPOLL_CTL_MOD, fd, &ev);
}

void epoll_init_sched_event(
                struct event_data *evt,
                sched_event_handler_t sched_handler,
                void *data)
{
    evt->sched_handler = sched_handler;
    evt->scheduled = 0;
    evt->data = data;
    INIT_LIST_HEAD(&evt->e_list);
}

void epoll_add_sched_event(struct event_data *evt)
{
    if (!evt->scheduled) {
        evt->scheduled = 1;
        list_add_tail(&evt->e_list, &epoll_sched_events_list);
    }
}

void epoll_remove_sched_event(struct event_data *evt)
{
    if (evt->scheduled) {
        evt->scheduled = 0;
        list_del_init(&evt->e_list);
    }
}

/* strcpy, while eating multiple white spaces */
void utils_spacecpy(char **dest, const char *src)
{
    const char *s = src;
    char *d = *dest;

    while (*s) {
        if (isspace(*s)) {
            if (!*(s+1))
                break;
            if (isspace(*(s+1))) {
                s++;
                continue;
            }
        }
        *d++ = *s++;
    }
    *d = '\0';
}

int utils_call_program(const char *cmd, void (*callback)(void *data, int result),
        void *data, char *output, int op_len, int flags)
{
    pid_t pid;
    int fds[2], ret, i;
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

    ret = pipe(fds);
    if (ret < 0) {
        TCP_SERVER_ERROR("pipe create failed for %s\n", cmd);
        return ret;
    }

    dprintf("%s, pipe: %d %d\n", cmd, fds[0], fds[1]);

    pid = fork();
    if (pid < 0) {
        TCP_SERVER_ERROR("fork failed for: %s\n", cmd);
        close(fds[0]);
        close(fds[1]);
        return pid;
    }

    if (!pid) {
        close(1);
        ret = dup(fds[1]);
        if (ret < 0) {
            TCP_SERVER_ERROR("dup failed for: %s\n", cmd);
            exit(-1);
        }
        close(fds[0]);
        execv(argv[0], argv);

        TCP_SERVER_ERROR("execv failed for: %s\n", cmd);
        exit(-1);
    } else {
        struct timeval tv;
        fd_set rfds;
        int ret_sel;

        close(fds[1]);
        prctl(PR_SET_NAME, "tgtd_callback", NULL, NULL, NULL);
        /* 0.1 second is okay, as the initiator will retry anyway */
        do {
            FD_ZERO(&rfds);
            FD_SET(fds[0], &rfds);
            tv.tv_sec = 0;
            tv.tv_usec = 100000;
            ret_sel = select(fds[0]+1, &rfds, NULL, NULL, &tv);
        } while (ret_sel < 0 && rcno == EINTR);
        if (ret_sel <= 0) { /* rcor or timeout */
            TCP_SERVER_ERROR("timeout on redirect callback, terminating "
                "child pid %d\n", pid);
            kill(pid, SIGTERM);
        }
        do {
            ret = waitpid(pid, &i, 0);
        } while (ret < 0 && rcno == EINTR);
        if (ret < 0) {
            TCP_SERVER_ERROR("waitpid failed for: %s\n", cmd);
            close(fds[0]);
            return ret;
        }
        if (ret_sel > 0) {
            ret = read(fds[0], output, op_len);
            if (ret < 0) {
                TCP_SERVER_ERROR("failed to get output from: %s\n", cmd);
                close(fds[0]);
                return ret;
            }
        }

        if (callback)
            callback(data, WEXITSTATUS(i));
        close(fds[0]);
    }

    return 0;
}

static LIST_HEAD(epoll_sched_events_list);

static int epoll_exec_scheduled(void)
{
    struct list_head *last_sched;
    struct event_data *tev, *tevn;
    int work_remains = 0;

    if (!list_empty(&epoll_sched_events_list)) {
        /* execute only work scheduled till now */
        last_sched = epoll_sched_events_list.prev;
        list_for_each_entry_safe(tev, tevn, &epoll_sched_events_list,
                     e_list) {
            tgt_remove_sched_event(tev);
            tev->sched_handler(tev);
            if (&tev->e_list == last_sched)
                break;
        }
        if (!list_empty(&epoll_sched_events_list))
            work_remains = 1;
    }
    return work_remains;
}

#define EPOLL_EVENT_MAX (1024)

static int epoll_event_loop(int ep_fd)
{
    int nevent = 0;
    int i = 0;
    int rc = 0;
    int sched_remains = 0;
    int timeout = 0;;
    struct epoll_event events[EPOLL_EVENT_MAX];
    struct event_data *tev;
    int events_num = 0;

    while(1) {
        sched_remains = epoll_exec_scheduled();
        timeout = sched_remains ? 0 : -1;
        nevent = epoll_wait(ep_fd, events, EPOLL_EVENT_MAX, timeout);
        if (nevent < 0) {
            rc = -errno;
            if (rc != -EINTR) {
                TCP_SERVER_ERROR("%d\n", rc);
                goto l_out;
            }
            continue;
        }
        
        for (i = 0; i < nevent; i++) {
            tev = (struct event_data *) events[i].data.ptr;
            tev->handler(tev->fd, events[i].events, tev->data);
        }
    }
}

int tcp_handler_lld_init_one(int lld_index)
{
    int rc;

    INIT_LIST_HEAD(&tcp_handler_drivers[lld_index]->target_list);
    if (tcp_handler_drivers[lld_index]->init) {
/** comment by hy 2020-09-19
 * # 这里调用了 init
     在运行前 有两个 驱动模块
     iscsi, rdma 的 iser
     这里以 iscsi 为例 调用 init = iscsi_init
     iscsi_init 调用 iscsi_transport 类型中的 ep_init
     iscsi_tcp iscsi_iser
     这里以 iscsi_tcp 为例 ep_init = iscsi_tcp_init
 */
        rc = tcp_handler_drivers[lld_index]->init(lld_index, spare_args);
        if (rc) {
            tcp_handler_drivers[lld_index]->drv_state = DRIVER_ERR;
            return rc;
        }
        tcp_handler_drivers[lld_index]->drv_state = DRIVER_INIT;
    }
    return 0;
}

static int tcp_handler_lld_init(void)
{
    int i, nr;

    for (i = nr = 0; tcp_handler_drivers[i]; i++) {
/** comment by hy 2020-09-20
 * # 处理准备连接超时外,核心逻辑 iscsi_tcp_event_handler
 */
        if (!tcp_handler_lld_init_one(i))
            nr++;
    }
    return nr;
}

static void tcp_handler_lld_exit(void)
{
    int i = 0;

    for (i = 0; tcp_handler_drivers[i]; i++) {
        if (tcp_handler_drivers[i]->exit)
            tcp_handler_drivers[i]->exit();
        tcp_handler_drivers[i]->drv_state = DRIVER_EXIT;
    }
}

struct tcp_handler_param {
    int (*parse_func)(char *);
    char *name;
};


#define TCP_HANDLER_PARAM_MAX (64)
static struct tcp_handler_param tcp_handler_param[TCP_HANDLER_PARAM_MAX];

int tcp_handler_setup_param(char *name, int (*parser)(char *))
{
    int i = 0;
    int rc = 0;

    for (i = 0; i < TCP_HANDLER_PARAM_MAX; i++) {
        if (!tcp_handler_param[i].name) {
            break;
        }
    }
    if (i >= TCP_HANDLER_PARAM_MAX) {
        rc = -1;
        goto l_out;
    }

    tcp_handler_param[i].name = name;
    tcp_handler_param[i].parse_func = parser;
    rc = 0;

l_out:
    return rc;
}

static int tcp_handler_parse_params(char *name, char *p)
{
    int i = 0;
    int rc = 0;

    for (i = 0; i < TCP_HANDLER_PARAM_MAX && tcp_handler_param[i].name; i++) {
        if (!strcmp(name, tcp_handler_param[i].name)) {
            rc = tcp_handler_param[i].parse_func(p);
            goto l_out;
        }
    }

    TCP_SERVER_ERROR("'%s' is an unknown option\n", name);
    rc = -1;
l_out:
    return rc;
}

int main(int argc, char **argv)
{
    struct sigaction sa_old;
    struct sigaction sa_new;
    int rc, ch, longindex, nr_lld = 0;
    int is_daemon = 1, is_debug = 0, use_logger = 1;
    int ret;
/* modify end by hy, 2020-09-20 添加默认的pid 文件 */
    char *pidfile = "/var/run/tgtd/tgtd.pid";
/* modify end by hy, 2020-09-20 */

    sa_new.sa_handler = signal_catch;
    sigemptyset(&sa_new.sa_mask);
    sa_new.sa_flags = 0;
    sigaction(SIGPIPE, &sa_new, &sa_old);
    sigaction(SIGTERM, &sa_new, &sa_old);

    pagesize = sysconf(_SC_PAGESIZE);
    for (pageshift = 0;; pageshift++)
        if (1UL << pageshift == pagesize)
            break;

    optrc = 0;

    while ((ch = getopt_long(argc, argv, short_options, long_options,
                 &longindex)) >= 0) {
        switch (ch) {
        case 'f':
            is_daemon = 0;
            use_logger = 1;
            break;
        case 'D':
            is_daemon = 0;
            break;
        case 'C':
/** comment by hy 2020-09-22
 * # 控制端口
 */
            ret = str_to_int_ge(optarg, control_port, 0);
            if (ret)
                bad_optarg(ret, ch, optarg);
            break;
        case 't':
            ret = str_to_int_gt(optarg, nr_iothreads, 0);
            if (ret)
                bad_optarg(ret, ch, optarg);
            break;
        case 'p':
            pidfile = strdup(optarg);
            if (pidfile == NULL) {
                TCP_SERVER_ERROR("can't duplicate pid file\n");
                exit(1);
            }
            break;
        case 'd':
            ret = str_to_int_range(optarg, is_debug, 0, 1);
            if (ret)
                bad_optarg(ret, ch, optarg);
            break;
        case 'V':
            version();
            break;
        case 'h':
            usage(0);
            break;
        default:
            if (strncmp(argv[optind - 1], "--", 2))
                usage(1);

            ret = parse_params(argv[optind - 1] + 2, argv[optind]);
            if (ret)
                usage(1);

            break;
        }
    }

    ep_fd = epoll_create(4096);
    if (ep_fd < 0) {
        TCP_SERVER_ERROR("can't create epoll fd\n");
        exit(1);
    }

    spare_args = optind < argc ? argv[optind] : NULL;

    if (is_daemon && daemon(0, 0))
        exit(1);

/** comment by hy 2020-09-20
 * # 处理管理套接字
     这里的核心函数 mgmt_event_handler ->mtask_recv_send_handler
     mtask_received 处理创建设备等
     concat_write
 */
    rc = ipc_init();
    if (rc)
        exit(1);
/* modify begin by hy, 2020-09-20, BugId:123 原因: 应该使用 use_logger */
    rc = log_init(program_name, LOG_SPACE_SIZE, use_logger, is_debug);
/* modify end by hy, 2020-09-20 */
    if (rc)
        exit(1);

/** comment by hy 2020-09-19
 * # 初始化 后端驱动初始化,其中的核心逻辑 iscsi_tcp_event_handler
     accpt 操作之后连接处理 的连接 iscsi_tcp_init
           iscsi_rx_handler
           iscsi_tx_handler
 */
    nr_lld = lld_init();
    if (!nr_lld) {
        TCP_SERVER_ERROR("No available low level driver!\n");
        exit(1);
    }

    rc = oom_adjust();
    if (rc && (rcno != EACCES) && getuid() == 0)
        exit(1);

    rc = nr_file_adjust();
    if (rc)
        exit(1);

/** comment by hy 2020-09-19
 * # 定时线程处理 tcp 检查链路
 */
    rc = work_timer_start();
    if (rc)
        exit(1);

/** comment by hy 2020-09-19
 * # 加载后端驱动注册模块,核心逻辑为 register_bs_module
 */
    bs_init();

#ifdef USE_SYSTEMD
    sd_notify(0, "READY=1\nSTATUS=Starting event loop...");
#endif

/* modify end by hy, 2020-09-20 不管是不是后台进程都创建pid文件 */
    if (pidfile)
        create_pid_file(pidfile);
/* modify end by hy, 2020-09-20 */

/** comment by hy 2020-09-20
 * # 监控到事件执行其对应的处理,主要可以看在上面连个核心函数
     上面进行包装回调放入队列中,这里进行执行回调
 */
    event_loop();

    lld_exit();

    work_timer_stop();

    ipc_exit();

    log_close();

/* modify begin by hy, 2020-09-20, BugId:123 原因: 不管是不是后台进程都创建pid文件 */
    if (pidfile)
        unlink(pidfile);
/* modify end by hy, 2020-09-20 */

    return 0;
}
