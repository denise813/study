#ifndef YSFS_LOG_H
#define YSFS_LOG_H


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


#define ysfs_log(level, format, args...) \
do{ \
    printf("%s:%d\t" format "\n",  __FILE__, __LINE__, ##args); \
}while(0)


#define YSFS_TRACE(format, args...)                                 \
do {                                                                                \
    snprintf(NULL, 0, format, ##args);                                           \
    if (ENABLE_LOG == TRUE) {                               \
        ysfs_log(FUSE_LOG_DEBUG, format, ##args);                              \
    }                                                                               \
} while (0)

#define YSFS_DEBUG(format, args...)                                 \
do {                                                                                \
    snprintf(NULL, 0, format, ##args);                                           \
    if (ENABLE_LOG == true) {                                                       \
        ysfs_log(FUSE_LOG_DEBUG, format, ##args);                              \
    }                                                                               \
} while (0)

#define YSFS_INFO(format, args...)                                  \
do {                                                                                \
    snprintf(NULL, 0, format, ##args);                                           \
    if (ENABLE_LOG == TRUE) {                                                       \
        ysfs_log(FUSE_LOG_INFO, format, ##args);                               \
    }                                                                               \
} while (0)

#define YSFS_WARN(format, args...)                                  \
do {                                                                                \
    snprintf(NULL, 0, format, ##args);                                           \
    if (ENABLE_LOG == true) {                                                       \
        ysfs_log(FUSE_LOG_WARNING, format, ##args);                               \
    }                                                                               \
} while (0)

#define YSFS_ERROR(format, args...)                                 \
do {                                                                                \
     snprintf(NULL, 0, format, ##args);                                          \
    if (ENABLE_LOG == TRUE) {                                                       \
        ysfs_log(FUSE_LOG_ERR, format, ##args);                              \
    }                                                                               \
} while (0)

#define YSFS_FATAL(format, args...)                                 \
do {                                                                                \
    snprintf(NULL, 0, format, ##args);                                           \
    if (ENABLE_LOG == TRUE) {                                                       \
        ysfs_log(FUSE_LOG_CRIT, format, ##args);                              \
    }                                                                               \
} while (0)



#ifdef __cplusplus
} /* extern "C" */
#endif


#endif
