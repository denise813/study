
#include "tcp_config.h"
#include "tcp_hander_driver.h"



static void event_loop(void)
{
	int nevent, i, sched_remains, timeout;
	struct epoll_event events[1024];
	struct event_data *tev;

retry:
	sched_remains = tgt_exec_scheduled();
	timeout = sched_remains ? 0 : -1;

/** comment by hy 2020-09-19
 * # 事件监控
     epoll将会把发生的事件复制到 events数组
 */
	nevent = epoll_wait(ep_fd, events, ARRAY_SIZE(events), timeout);
	if (nevent < 0) {
		if (errno != EINTR) {
			eprintf("%m\n");
			exit(1);
		}
	} else if (nevent) {
		for (i = 0; i < nevent; i++) {
			tev = (struct event_data *) events[i].data.ptr;
/** comment by hy 2020-09-20
 * # 这里对应的就是其回调函数
 */
			tev->handler(tev->fd, events[i].events, tev->data);

			if (event_need_refresh) {
				event_need_refresh = 0;
				goto retry;
			}
		}
	}

	if (system_active)
		goto retry;
}

int lld_init_one(int lld_index)
{
	int err;

	INIT_LIST_HEAD(&tgt_drivers[lld_index]->target_list);
	if (tgt_drivers[lld_index]->init) {
/** comment by hy 2020-09-19
 * # 这里调用了 init
     在运行前 有两个 驱动模块
     iscsi, rdma 的 iser
     这里以 iscsi 为例 调用 init = iscsi_init
     iscsi_init 调用 iscsi_transport 类型中的 ep_init
     iscsi_tcp iscsi_iser
     这里以 iscsi_tcp 为例 ep_init = iscsi_tcp_init
 */
		err = tgt_drivers[lld_index]->init(lld_index, spare_args);
		if (err) {
			tgt_drivers[lld_index]->drv_state = DRIVER_ERR;
			return err;
		}
		tgt_drivers[lld_index]->drv_state = DRIVER_INIT;
	}
	return 0;
}

static int lld_init(void)
{
	int i, nr;

	for (i = nr = 0; tgt_drivers[i]; i++) {
/** comment by hy 2020-09-20
 * # 处理准备连接超时外,核心逻辑 iscsi_tcp_event_handler
 */
		if (!lld_init_one(i))
			nr++;
	}
	return nr;
}

static void lld_exit(void)
{
	int i;

	for (i = 0; tgt_drivers[i]; i++) {
		if (tgt_drivers[i]->exit)
			tgt_drivers[i]->exit();
		tgt_drivers[i]->drv_state = DRIVER_EXIT;
	}
}

struct tgt_param {
	int (*parse_func)(char *);
	char *name;
};

static struct tgt_param params[64];

int setup_param(char *name, int (*parser)(char *))
{
	int i;

	for (i = 0; i < ARRAY_SIZE(params); i++)
		if (!params[i].name)
			break;

	if (i < ARRAY_SIZE(params)) {
		params[i].name = name;
		params[i].parse_func = parser;

		return 0;
	} else
		return -1;
}

static int parse_params(char *name, char *p)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(params) && params[i].name; i++) {
		if (!strcmp(name, params[i].name))
			return params[i].parse_func(p);
	}

	fprintf(stderr, "'%s' is an unknown option\n", name);

	return -1;
}

int main(int argc, char **argv)
{
	struct sigaction sa_old;
	struct sigaction sa_new;
	int err, ch, longindex, nr_lld = 0;
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

	opterr = 0;

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
				fprintf(stderr, "can't duplicate pid file\n");
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
		fprintf(stderr, "can't create epoll fd, %m\n");
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
	err = ipc_init();
	if (err)
		exit(1);
/* modify begin by hy, 2020-09-20, BugId:123 原因: 应该使用 use_logger */
	err = log_init(program_name, LOG_SPACE_SIZE, use_logger, is_debug);
/* modify end by hy, 2020-09-20 */
	if (err)
		exit(1);

/** comment by hy 2020-09-19
 * # 初始化 后端驱动初始化,其中的核心逻辑 iscsi_tcp_event_handler
     accpt 操作之后连接处理 的连接 iscsi_tcp_init
           iscsi_rx_handler
           iscsi_tx_handler
 */
	nr_lld = lld_init();
	if (!nr_lld) {
		fprintf(stderr, "No available low level driver!\n");
		exit(1);
	}

	err = oom_adjust();
	if (err && (errno != EACCES) && getuid() == 0)
		exit(1);

	err = nr_file_adjust();
	if (err)
		exit(1);

/** comment by hy 2020-09-19
 * # 定时线程处理 tcp 检查链路
 */
	err = work_timer_start();
	if (err)
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
