#ifndef FUSEFS_LOG_H
#define FUSEFS_LOG_H


#include <stdio.h>


#ifdef __cplusplus
extern "C"
{
#endif


#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif


#ifndef ENABLE_LOG
#define ENABLE_LOG (TRUE)
#endif


#define fusefs_log(level, format, args...) \
do{ \
    printf("%s:%d\t" format "\n",  __FILE__, __LINE__, ##args); \
}while(0)


#define FUSEFS_TRACE(format, args...)                                 \
do {                                                                                \
    snprintf(NULL, 0, format, ##args);                                           \
    if (ENABLE_LOG == TRUE) {                               \
        fusefs_log(FUSE_LOG_DEBUG, format, ##args);                              \
    }                                                                               \
} while (0)

#define FUSEFS_DEBUG(format, args...)                                 \
do {                                                                                \
    snprintf(NULL, 0, format, ##args);                                           \
    if (ENABLE_LOG == TRUE) {                                                       \
        fusefs_log(FUSE_LOG_DEBUG, format, ##args);                              \
    }                                                                               \
} while (0)

#define FUSEFS_INFO(format, args...)                                  \
do {                                                                                \
    snprintf(NULL, 0, format, ##args);                                           \
    if (ENABLE_LOG == TRUE) {                                                       \
        fusefs_log(FUSE_LOG_INFO, format, ##args);                               \
    }                                                                               \
} while (0)

#define FUSEFS_WARN(format, args...)                                  \
do {                                                                                \
    snprintf(NULL, 0, format, ##args);                                           \
    if (ENABLE_LOG == TRUE) {                                                       \
        fusefs_log(FUSE_LOG_WARNING, format, ##args);                               \
    }                                                                               \
} while (0)

#define FUSEFS_ERROR(format, args...)                                 \
do {                                                                                \
     snprintf(NULL, 0, format, ##args);                                          \
    if (ENABLE_LOG == TRUE) {                                                       \
        fusefs_log(FUSE_LOG_ERR, format, ##args);                              \
    }                                                                               \
} while (0)

#define FUSEFS_FATAL(format, args...)                                 \
do {                                                                                \
    snprintf(NULL, 0, format, ##args);                                           \
    if (ENABLE_LOG == TRUE) {                                                       \
        fusefs_log(FUSE_LOG_CRIT, format, ##args);                              \
    }                                                                               \
} while (0)



#ifdef __cplusplus
} /* extern "C" */
#endif


#endif
