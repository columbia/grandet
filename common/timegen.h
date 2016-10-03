#ifndef _GRANDET_TIMEGEN_H_
#define _GRANDET_TIMEGEN_H_

#include <sys/time.h>

class TimeGen {
public:
    static inline time_t now() {
        struct timeval tv;
        gettimeofday(&tv, NULL);
        return tv.tv_sec;
    }
};

#endif // _GRANDET_TIMEGEN_H_
