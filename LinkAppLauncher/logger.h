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
    Logger(CString& filePath) : logFilePath_(filePath) {
        // Open file to check if it exists
        CFile file;
        if (!file.Open(logFilePath_, CFile::modeCreate | CFile::modeWrite | CFile::modeNoTruncate)) {
            TRACE(_T("Unable to open log file: %s"), (LPCTSTR)filePath);
        }
        file.Close();
    }
    void log(LPCTSTR message) {
        try {
            std::lock_guard<std::mutex> lock(mutex_);

            CFile file;
            if (file.Open(logFilePath_, CFile::modeWrite | CFile::modeNoTruncate)) {
                file.SeekToEnd();

                // Get timestamp
                CString timeStamp(getCurrentTime().c_str());

                // Prepare the complete log entry
                CString completeLog;
                completeLog.Format(_T("%s - %s\r\n"), timeStamp, message);

                // Convert to UTF-8
#ifndef _UNICODE
                int nLen = MultiByteToWideChar(CP_ACP, 0, completeLog, -1, NULL, 0);
                wchar_t* pszW = new wchar_t[nLen];
                MultiByteToWideChar(CP_ACP, 0, completeLog, -1, pszW, nLen);
#else
                const wchar_t* pszW = completeLog;
                int nLen = completeLog.GetLength() + 1;
#endif

                // Convert to UTF-8
                int utf8Len = WideCharToMultiByte(CP_UTF8, 0, pszW, -1, NULL, 0, NULL, NULL);
                if (utf8Len > 0) {
                    char* utf8Buffer = new char[utf8Len];
                    WideCharToMultiByte(CP_UTF8, 0, pszW, -1, utf8Buffer, utf8Len, NULL, NULL);

                    // Write UTF-8 data (excluding null terminator)
                    file.Write(utf8Buffer, utf8Len - 1);

                    delete[] utf8Buffer;
                }

#ifndef _UNICODE
                delete[] pszW;
#endif

                file.Close();
            }
        }
        catch (CFileException* e) {
            TRACE(_T("Failed to write to log file: %d\n"), e->m_cause);
            e->Delete();
        }
    }

private:
    CString logFilePath_;
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
        oss << '.' << std::setw(3) << std::setfill('0') << milliseconds.count();

        return oss.str();
    }
};