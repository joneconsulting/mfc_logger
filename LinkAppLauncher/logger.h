#include <iostream>
#include <fstream>
#include <string>
#include <mutex>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <memory>

class Logger {
public:
    Logger() {}
    Logger(CString& filePath) : logFile(filePath, std::ios::app) {
        if (!logFile.is_open()) {
           TRACE(_T("Unable to open log file: %s"), (LPCTSTR)filePath);
        }
    }
    void log(LPCTSTR message) {
        if (!logFile.is_open())
            return;
        USES_CONVERSION;
        std::lock_guard<std::mutex> lock(mutex_);
        std::string outstr;
        outstr.assign(T2A(message));
        logFile << getCurrentTime() << " - " << outstr << std::endl;
        if (!logFile) {
            TRACE(_T("Failed to write to log file."));
        }
    }

private:
    std::ofstream logFile;
    std::mutex mutex_;

    std::string getCurrentTime() {
        auto now = std::chrono::system_clock::now();
        auto now_time_t = std::chrono::system_clock::to_time_t(now);
        auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
        auto epoch = now_ms.time_since_epoch();
        auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(epoch) % 1000;

        std::tm now_tm;
#ifdef _WIN32
        localtime_s(&now_tm, &now_time_t);
#else
        localtime_r(&now_time_t, &now_tm);
#endif
        std::ostringstream oss;
        oss << std::put_time(&now_tm, "%Y-%m-%d %H:%M:%S");
        oss << '.' << std::setw(3) << std::setfill('0') << milliseconds.count();  // Add milliseconds

        return oss.str();
    }
};