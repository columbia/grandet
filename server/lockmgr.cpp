#include "lockmgr.h"
#include "log.h"
#include "key.h"
using namespace std;

bool LockMgr::lock(const Key& key, ConnInfo* stream) {
    lock_guard<mutex> lock(giant);

    auto it = locks.find(key);
    if (it == locks.end()) {
        locks.insert(make_pair(key, LockInfo()));
        return true;
    } else {
        it->second.waiters.push(stream);
        return false;
    }
}

ConnInfo* LockMgr::unlock(const Key& key) {
    lock_guard<mutex> lock(giant);

    auto it = locks.find(key);
    if (it == locks.end()) {
        Warn() << "unlocking unlocked key: " << key.toString() << endl;
        return nullptr;
    } else {
        if (it->second.waiters.empty()) {
            locks.erase(it);
            return nullptr;
        } else {
            ConnInfo *wake = it->second.waiters.front();
            it->second.waiters.pop();
            return wake;
        }
    }
}

bool LockMgr::isLocked(const Key& key) {
    lock_guard<mutex> lock(giant);

    auto it = locks.find(key);
    return (it != locks.end());
}

void LockMgr::reset() {
    lock_guard<mutex> lock(giant);

    if (!locks.empty()) {
        Warn() << "reset with keys locked! count: " << locks.size() << endl;
    }
    locks.clear();
}
