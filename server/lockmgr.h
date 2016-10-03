#ifndef _GRANDET_LOCKMGR_H_
#define _GRANDET_LOCKMGR_H_

#include <map>
#include <queue>
#include <mutex>

class Key;
class ConnInfo;

class LockMgr {
    struct LockInfo {
        std::queue<ConnInfo*> waiters;
    };

    std::map<Key, LockInfo> locks;
    std::mutex giant;
public:
    // Lock a key
    // #return
    //   true: locked
    //   false: need wait, OWN(stream)
    bool lock(const Key& key, ConnInfo* stream);
    // Unlock a key
    // #return
    //   null: no waiter
    //   non-null: has waiter, RELEASE(#return)
    ConnInfo* unlock(const Key& key);
    // !!!For Debug Use!!!
    // Check if a key is locked
    bool isLocked(const Key& key);

    void reset();
};

#endif // _GRANDET_LOCKMGR_H_
