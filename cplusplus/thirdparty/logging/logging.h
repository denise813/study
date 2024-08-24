#ifndef ZADP_SO_LOG_H_
#define ZADP_SO_LOG_H_


#include <stdio.h>



#define ZADP_SO_LOG_TRUE 1

#ifndef ENABLE_ZADP_SO_LOG
#define ZADP_SO_ENABLE_LOG 0
#else
#define ZADP_SO_ENABLE_LOG 1
#endif


enum ZADP_SO_LOG_LEVEL
{
    ZADP_SO_LOG_CRIT = 0,
    ZADP_SO_LOG_ERR,
    ZADP_SO_LOG_INFO,
    ZADP_SO_LOG_DEBUG,
    ZADP_SO_LOG_TRACE,
};


static int g_level = ZADP_SO_LOG_INFO;


#define zadp_so_log(level, format, args...) \
do{ \
    if (level <= g_level) { \
        printf("zadp_so %s:%d:level(%d)" format "\n",  __FILE__, __LINE__, level, ##args); \
    } \
}while(0)


#define ZADP_SO_TRACE(format, args...)                                 \
do {                                                                                \
    snprintf(NULL, 0, format, ##args);                                           \
    if (ZADP_SO_ENABLE_LOG == ZADP_SO_LOG_TRUE) {                               \
        zadp_so_log(ZADP_SO_LOG_TRACE, format, ##args);                              \
    }                                                                               \
} while (0)

#define ZADP_SO_DEBUG(format, args...)                                 \
do {                                                                                \
    snprintf(NULL, 0, format, ##args);                                           \
    if (ZADP_SO_ENABLE_LOG == ZADP_SO_LOG_TRUE) {                                                       \
        zadp_so_log(ZADP_SO_LOG_DEBUG, format, ##args);                              \
    }                                                                               \
} while (0)

#define ZADP_SO_INFO(format, args...)                                  \
do {                                                                                \
    snprintf(NULL, 0, format, ##args);                                           \
    if (ZADP_SO_ENABLE_LOG == ZADP_SO_LOG_TRUE) {                                                       \
        zadp_so_log(ZADP_SO_LOG_INFO, format, ##args);                               \
    }                                                                               \
} while (0)

#define ZADP_SO_WARN(format, args...)                                  \
do {                                                                                \
    snprintf(NULL, 0, format, ##args);                                           \
    if (ZADP_SO_ENABLE_LOG == ZADP_SO_LOG_TRUE) {                                                       \
        zadp_so_log(ZADP_SO_LOG_WARNING, format, ##args);                               \
    }                                                                               \
} while (0)

#define ZADP_SO_ERROR(format, args...)                                 \
do {                                                                                \
     snprintf(NULL, 0, format, ##args);                                          \
    if (ZADP_SO_ENABLE_LOG == ZADP_SO_LOG_TRUE) {                                                       \
        zadp_so_log(ZADP_SO_LOG_ERR, format, ##args);                              \
    }                                                                               \
} while (0)

#define ZADP_SO_FATAL(format, args...)                                 \
do {                                                                                \
    snprintf(NULL, 0, format, ##args);                                           \
    if (ZADP_SO_ENABLE_LOG == ZADP_SO_LOG_TRUE) {                                                       \
        zadp_so_log(ZADP_SO_LOG_CRIT, format, ##args);                              \
    }                                                                               \
} while (0)



#endif  // SRC_INCLUDE_ZBS_COMMON_H_
