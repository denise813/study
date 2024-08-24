#ifndef JNI_LOG_H_
#define JNI_LOG_H_


#include <stdio.h>
#include "logging.h"


#define JNI_LOG_TRUE 1

#ifndef ENABLE_JNI_LOG
#define JNI_ENABLE_LOG 0
#else
#define JNI_ENABLE_LOG 1
#endif


#define jni_log(level, format, args...) \
do{ \
    logging_log(level, "jni", format, ##args); \
}while(0)


#define JNI_TRACE(format, args...)                                 \
do {                                                                                \
    snprintf(NULL, 0, format, ##args);                                           \
    if (JNI_ENABLE_LOG == JNI_LOG_TRUE) {                               \
        jni_log(LOGGING_LOG_TRACE, format, ##args);                              \
    }                                                                               \
} while (0)

#define JNI_DEBUG(format, args...)                                 \
do {                                                                                \
    snprintf(NULL, 0, format, ##args);                                           \
    if (JNI_ENABLE_LOG == JNI_LOG_TRUE) {                                                       \
        jni_log(LOGGING_LOG_DEBUG, format, ##args);                              \
    }                                                                               \
} while (0)

#define JNI_INFO(format, args...)                                  \
do {                                                                                \
    snprintf(NULL, 0, format, ##args);                                           \
    if (JNI_ENABLE_LOG == JNI_LOG_TRUE) {                                                       \
        jni_log(LOGGING_LOG_INFO, format, ##args);                               \
    }                                                                               \
} while (0)

#define JNI_WARN(format, args...)                                  \
do {                                                                                \
    snprintf(NULL, 0, format, ##args);                                           \
    if (JNI_ENABLE_LOG == JNI_LOG_TRUE) {                                                       \
        jni_log(LOGGING_LOG_WARN, format, ##args);                               \
    }                                                                               \
} while (0)

#define JNI_ERROR(format, args...)                                 \
do {                                                                                \
     snprintf(NULL, 0, format, ##args);                                          \
    if (JNI_ENABLE_LOG == JNI_LOG_TRUE) {                                                       \
        jni_log(LOGGING_LOG_ERR, format, ##args);                              \
    }                                                                               \
} while (0)

#define JNI_FATAL(format, args...)                                 \
do {                                                                                \
    snprintf(NULL, 0, format, ##args);                                           \
    if (JNI_ENABLE_LOG == JNI_LOG_TRUE) {                                                       \
        jni_log(LOGGING_LOG_CRIT, format, ##args);                              \
    }                                                                               \
} while (0)


#endif  // SRC_INCLUDE_ZBS_COMMON_H_
