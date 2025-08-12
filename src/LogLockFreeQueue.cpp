#include "LogLockFreeQueue.h"
#include <cstdarg>
#include <ctime>
#include <sstream>
#include <iomanip>

LogLockFreeQueue::LogLockFreeQueue():m_logQueue(65536) {
}

LogLockFreeQueue::~LogLockFreeQueue() {
}

// 格式化日志消息（支持变参）
void LogLockFreeQueue::push(const char* format, ...) {
    // 1. 格式化用户消息
    va_list args;
    va_start(args, format);
    char user_msg[1024*32];
    int len = vsnprintf(user_msg, sizeof(user_msg), format, args);
    va_end(args);

    std::string msg_body = (len >= 0) ? std::string(user_msg, len) : std::string("Invalid format");

    std::tm local_time;
    std::time_t now = std::time(0);
    localtime_r(&now, &local_time);

    char buffer[32] = { 0 };
    strftime(buffer, sizeof(buffer), "[%Y-%m-%d %H:%M:%S]", &local_time);
    std::string time_str(buffer);

    // 2. 组合完整日志：时间 + 日志内容
    std::string full_msg = time_str + msg_body;

    // 3. 分配堆内存并入队
    std::string* msg_ptr = new std::string(std::move(full_msg));

    int retry_count = 0;
    while (!m_logQueue.push(msg_ptr)) {
        retry_count++;
        if (retry_count < 1000) {
            usleep(1000);  // 1ms
        } else {
            delete msg_ptr;
            break;
        }
    }
}

bool LogLockFreeQueue::pop(std::string& message)
{
    std::string* msg_ptr = nullptr;
    if (m_logQueue.pop(msg_ptr)) {
        message = std::move(*msg_ptr);
        delete msg_ptr;
        return true;
    }
    return false;
}