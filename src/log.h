#ifndef __LOG_H__
#define __LOG_H__

#include <cstring>
#include "LogLockFreeQueue.h"

enum LogLevel {
	LOG_LEVEL_TRACE = 0,
	LOG_LEVEL_DEBUG = 1,
	LOG_LEVEL_INFO  = 2,
	LOG_LEVEL_WARN  = 3,
	LOG_LEVEL_ERROR = 4,
    LOG_LEVEL_FINAL = 999
};

extern LogLockFreeQueue g_logger;
extern LogLevel g_logLevel;

// 内部日志宏（带级别判断）
#define LOG_LOG(level, level_name, msg, ...) \
    do { \
        if (level >= g_logLevel) { \
            g_logger.push("[%s][%s]: " msg, level_name, __func__, ##__VA_ARGS__); \
        } \
    } while(0)

// 外部接口
#define LOG_TRACE(msg, ...) \
    LOG_LOG(LOG_LEVEL_TRACE, "TRACE", msg, ##__VA_ARGS__)

#define LOG_DEBUG(msg, ...) \
    LOG_LOG(LOG_LEVEL_DEBUG, "DEBUG", msg, ##__VA_ARGS__)

#define LOG_INFO(msg, ...) \
    LOG_LOG(LOG_LEVEL_INFO,  "INFO ", msg, ##__VA_ARGS__)

#define LOG_WARN(msg, ...) \
    LOG_LOG(LOG_LEVEL_WARN,  "WARN ", msg, ##__VA_ARGS__)

#define LOG_ERROR(msg, ...) \
    LOG_LOG(LOG_LEVEL_ERROR, "ERROR", msg, ##__VA_ARGS__)

#endif //__LOG_H__