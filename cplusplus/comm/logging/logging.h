#ifndef LOGGING_LOG_H_
#define LOGGING_LOG_H_


#include <stdio.h>


#define LOGGING_LOG_TRUE 1

#ifndef ENABLE_LOGGING_LOG
#define LOGGING_ENABLE_LOG 0
#else
#define LOGGING_ENABLE_LOG 1
#endif


#ifdef __cplusplus
extern "C" {
#endif


enum LOGGING_LOG_LEVEL
{
    LOGGING_LOG_CRIT = 0,
    LOGGING_LOG_ERR,
    LOGGING_LOG_WARN,
    LOGGING_LOG_IMP,
    LOGGING_LOG_INFO,
    LOGGING_LOG_DEBUG,
    LOGGING_LOG_TRACE,
};


extern int g_logging_level;

#define logging_log(level, module, format, args...) \
do{ \
    if (LOGGING_ENABLE_LOG == LOGGING_LOG_TRUE) {                               \
        if (level <= g_logging_level) { \
            printf("[%s] %s:%d:level(%d)" format "\n", module, __FILE__, __LINE__, level, ##args); \
        } \
    } \
}while(0)


#define LOGGING_SET_LEVEL(level) \
do { \
    g_logging_level = level; \
}while(0)


#define LOGGING_TRACE(module, format, args...)                                 \
do {                                                                                \
    snprintf(NULL, 0, format, ##args);                                           \
    logging_log(LOGGING_LOG_TRACE, module, format, ##args);                              \
} while (0)

#define LOGGING_DEBUG(module, format, args...)                                 \
do {                                                                                \
    snprintf(NULL, 0, format, ##args);                                           \
    logging_log(LOGGING_LOG_DEBUG, format, ##args);                              \
} while (0)

#define LOGGING_INFO(module, format, args...)                                  \
do {                                                                                \
    snprintf(NULL, 0, format, ##args);                                           \
    logging_log(LOGGING_LOG_INFO, format, ##args);                               \
} while (0)

#define LOGGING_IMP(module, format, args...)                                  \
do {                                                                                \
    snprintf(NULL, 0, format, ##args);                                           \
    logging_log(LOGGING_LOG_IMP, format, ##args);                               \
} while (0)

#define LOGGING_WARN(module, format, args...)                                  \
do {                                                                                \
    snprintf(NULL, 0, format, ##args);                                           \
    logging_log(LOGGING_LOG_WARN, module, format, ##args);                               \
} while (0)

#define LOGGING_ERROR(module, format, args...)                                 \
do {                                                                                \
    snprintf(NULL, 0, format, ##args);                                          \
    logging_log(LOGGING_LOG_ERR, module, format, ##args);                              \
} while (0)

#define LOGGING_FATAL(module, format, args...)                                 \
do {                                                                                \
    snprintf(NULL, 0, format, ##args);                                           \
    logging_log(LOGGING_LOG_CRIT, module, format, ##args);                              \
} while (0)


#ifdef __cplusplus
    }
#endif


#endif  // SRC_INCLUDE_ZBS_COMMON_H_
