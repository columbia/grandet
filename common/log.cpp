#include "log.h"
#include <fstream>
#include <libgen.h>
#include <cstring>
#include <iomanip>
#include <sys/time.h>
#include <mutex>

using namespace std;

ostream* logger = nullptr;
bool withSrc = false;
bool withTime = true;
LogLevel logLevel = LOG_DEF;
mutex logMutex;

onullstream nullStream;

void LogToFile(string& filename) {
    logger = new fstream(filename);
}

void LogToConsole() {
    if (logger) {
        delete logger;
    }
    logger = nullptr;
}

void SetLogLevel(LogLevel level) {
    logLevel = level;
}
