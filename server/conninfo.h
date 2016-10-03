#ifndef _GRANDET_CONNINFO_H_
#define _GRANDET_CONNINFO_H_

#include "key.h"
#include "network.h"

#include <set>

class ConnInfo {
    std::set<Key> lockedKeys;

public:
    virtual flow_t* getStream() = 0;

    void addLocked(const Key& key) {
        lockedKeys.insert(key);
    }

    void removeLocked(const Key& key) {
        lockedKeys.erase(key);
    }

    bool locked(const Key& key) {
        return lockedKeys.find(key) != lockedKeys.end();
    }

    const std::set<Key>& getLockedKeys() {
        return lockedKeys;
    }

    virtual ~ConnInfo() {}
};

#endif // _GRANDET_CONNINFO_H_
