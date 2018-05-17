#pragma once

#include <memory>
#include <vector>

class LogManager {
public:
    static LogManager& getLogger();
    void logFatal(const char* pMessage);
    void logWarning(const char* pMessage);
private:
    LogManager();
};