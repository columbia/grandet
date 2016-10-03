#ifndef _GRANDET_LOG_H_
#define _GRANDET_LOG_H_

#include <iostream>
#include <string>
#include <mutex>
#include <iomanip>
#include <cstring>
#include <sys/time.h>

enum LogLevel {
    LOG_DEF,
    LOG_DBG,
    LOG_INFO,
    LOG_WARN,
    LOG_ERR
};

class nullbuf: public std::streambuf {
    int overflow(int c) override
    {
        return false; // indicate success
    }
};

class onullstream: public std::ostream {
    public:
        onullstream(): std::ios(&m_sbuf), std::ostream(&m_sbuf)
    {
        // note: the original code is missing the required this->
        this->init(&m_sbuf);
    }

    private:
        nullbuf m_sbuf;
};

extern std::mutex logMutex;
extern LogLevel logLevel;
extern onullstream nullStream;
extern std::ostream* logger;
extern bool withSrc;
extern bool withTime;

class RealLog : public std::ostream {
    std::ostream *dest;

public:
    template<class T>
    RealLog& operator<<(const T& data) {
        (*dest) << data;
        return *this;
    }

    RealLog& operator<<(std::ostream& (*F)(std::ostream&)) {
        F(*dest);
        return *this;
    }

    ~RealLog() {
        logMutex.unlock();
    }

    RealLog(const char* file, const char* func, int line, LogLevel level) {
        logMutex.lock();
        if (level < logLevel) {
            dest = &nullStream;
            return;
        }

        if (logger == nullptr) {
            dest = &std::cerr;
        } else {
            dest = logger;
        }
        if (withTime) {
            struct timeval tv;
            gettimeofday(&tv, NULL);
            (*dest) << tv.tv_sec << '.' << std::setw(6) << std::setfill('0') << tv.tv_usec << " ";
        }
        if (withSrc) {
            char *filename = strdup(file);
            (*dest) << basename(filename) << ":" << line << " <" << func << ">:";
            free(filename);
        }
        switch (level) {
            case LOG_ERR:
                (*dest) << " [ERROR] ";
                break;
            case LOG_WARN:
                (*dest) << " [WARN ] ";
                break;
            case LOG_INFO:
                (*dest) << " [INFO ] ";
                break;
            case LOG_DBG:
                (*dest) << " [DEBUG] ";
                break;
            case LOG_DEF:
                break;
        }
    }
};

#define Log() RealLog(__FILE__, __func__, __LINE__, LOG_DEF)
#define Err() RealLog(__FILE__, __func__, __LINE__, LOG_ERR)
#define Warn() RealLog(__FILE__, __func__, __LINE__, LOG_WARN)
#define Dbg() RealLog(__FILE__, __func__, __LINE__, LOG_DBG)
#define Info() RealLog(__FILE__, __func__, __LINE__, LOG_INFO)
void LogToFile(std::string& filename);
void LogToConsole();
void SetLogLevel(LogLevel level);

#endif // _GRANDET_LOG_H_
