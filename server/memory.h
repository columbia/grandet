#ifndef _GRANDET_MEMORY_H_
#define _GRANDET_MEMORY_H_

#include <redis3m/simple_pool.h>
#include <map>
#include <mutex>

#include "key.h"

class Memory {
    typedef int choice_id;
    std::map<Key, choice_id> data;
    bool persist;
    std::mutex lock;

    redis3m::simple_pool::ptr_t pool;

public:
    choice_id remember(const Key& key, const choice_id& choice);
    bool recall(const Key& key, choice_id* choice);
    void forget(const Key& key);

    Memory();
    void load();
    // !!!For Debug Use!!!
    // Return information about all keys and choices
    const std::map<Key, choice_id>& dump();
    void lockIt();
    void unlockIt();
};

#endif // _GRANDET_MEMORY_H_
