#ifndef _GRANDET_STATISTIC_H_
#define _GRANDET_STATISTIC_H_

#include <string>
#include <map>

class Key;

class Statistic {
    struct Entry {
        long put, get;
    };
    std::map<std::string, Entry> data;

public:
    void notifyGet(const Key& key);
    void notifyPut(const Key& key);

    void getInfo(const Key& key, long& get, long& put);
    void reset();
};

#endif // _GRANDET_STATISTIC_H_
