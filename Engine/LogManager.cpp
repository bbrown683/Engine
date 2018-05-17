#include "LogManager.hpp"

#define LOGURU_IMPLEMENTATION 1
#include "ThirdParty/loguru.hpp"

LogManager& LogManager::getLogger() {
    static LogManager instance;
    return instance;
}

void LogManager::logFatal(const char* pMessage) {
    LOG_F(FATAL, pMessage);
}

void LogManager::logWarning(const char* pMessage) {
    LOG_F(WARNING, pMessage);
}

LogManager::LogManager() {
    loguru::add_file("error_log.log", loguru::FileMode::Append, loguru::Verbosity_MAX);
}
