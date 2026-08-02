#pragma once
#include "imgui/imgui.h"
#include <vector>
#include <string>
#include <mutex>
#include <cstdarg>

enum class LogLevel {
    Info,
    Warning,
    Error,
    Performance
};

struct LogEntry {
    LogLevel level;
    std::string timestamp;
    std::string message;
};

class ConsoleLog {
private:
    std::vector<LogEntry> entries;
    std::mutex logMutex;

    bool autoScroll;
    bool showInfo;
    bool showWarning;
    bool showError;
    bool showPerformance;
    bool isCollapsed;
    float height;
    char filterBuffer[256];

    int infoCount;
    int warningCount;
    int errorCount;
    int performanceCount;

    ConsoleLog();

public:
    static ConsoleLog& Get();

    void AddLog(LogLevel level, const char* fmt, ...);
    void Clear();
    void Draw(const char* title = "Console Log");

    bool IsCollapsed() const { return isCollapsed; }
    void SetCollapsed(bool collapsed) { isCollapsed = collapsed; }
};

void RaylibTraceLogCallback(int logLevel, const char *text, va_list args);
