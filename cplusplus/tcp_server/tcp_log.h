#ifndef __TCP_SERVER_LOG_H_
#define __TCP_SERVER_LOG_H_


#define TCP_SERVER_LOG_TRUE true

#ifndef ENABLE_LOG
#define ENABLE_LOG TCP_SERVER_LOG_TRUE
#endif


#define tcp_server_log(level, format, args...) \
do{ \
    printf("%s:%d\t" format "\n",  __FILE__, __LINE__, ##args); \
}while(0)


#define TCP_SERVER_TRACE(format, args...)                                 \
do {                                                                                \
    snprintf(NULL, 0, format, ##args);                                           \
    if (ENABLE_LOG == TCP_SERVER_LOG_TRUE) {                               \
        tcp_server_log(FUSE_LOG_DEBUG, format, ##args);                              \
    }                                                                               \
} while (0)

#define TCP_SERVER_DEBUG(format, args...)                                 \
do {                                                                                \
    snprintf(NULL, 0, format, ##args);                                           \
    if (ENABLE_LOG == TCP_SERVER_LOG_TRUE) {                                                       \
        tcp_server_log(FUSE_LOG_DEBUG, format, ##args);                              \
    }                                                                               \
} while (0)

#define TCP_SERVER_INFO(format, args...)                                  \
do {                                                                                \
    snprintf(NULL, 0, format, ##args);                                           \
    if (ENABLE_LOG == TCP_SERVER_LOG_TRUE) {                                                       \
        tcp_server_log(FUSE_LOG_INFO, format, ##args);                               \
    }                                                                               \
} while (0)

#define TCP_SERVER_WARN(format, args...)                                  \
do {                                                                                \
    snprintf(NULL, 0, format, ##args);                                           \
    if (ENABLE_LOG == TCP_SERVER_LOG_TRUE) {                                                       \
        tcp_server_log(FUSE_LOG_WARNING, format, ##args);                               \
    }                                                                               \
} while (0)

#define TCP_SERVER_ERROR(format, args...)                                 \
do {                                                                                \
     snprintf(NULL, 0, format, ##args);                                          \
    if (ENABLE_LOG == TCP_SERVER_LOG_TRUE) {                                                       \
        tcp_server_log(FUSE_LOG_ERR, format, ##args);                              \
    }                                                                               \
} while (0)

#define TCP_SERVER_FATAL(format, args...)                                 \
do {                                                                                \
    snprintf(NULL, 0, format, ##args);                                           \
    if (ENABLE_LOG == TCP_SERVER_LOG_TRUE) {                                                       \
        tcp_server_log(FUSE_LOG_CRIT, format, ##args);                              \
    }                                                                               \
} while (0)


#endif
