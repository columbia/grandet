#ifndef _GRANDET_CONNPOOL_H_
#define _GRANDET_CONNPOOL_H_

#include <vector>
#include <mutex>
#include <condition_variable>

template <class ConnPtr>
class ConnPool {
    std::mutex lock;
    std::condition_variable condition;
    std::vector<ConnPtr> conns;

public:
    ConnPool(int size = 0) {
        conns.resize(size);
    }

    ConnPtr get() {
        std::unique_lock<std::mutex> guard(lock);
        while (conns.size() == 0) {
            condition.wait(guard);
        }
        ConnPtr conn = conns.back();
        conns.pop_back();
        return conn;
    }

    void put(ConnPtr conn) {
        std::unique_lock<std::mutex> guard(lock);
        conns.push_back(conn);
        condition.notify_one();
    }
};

template <class ConnPtr>
class PoolConn {
    ConnPool<ConnPtr> *pool;
    ConnPtr conn;

public:
    PoolConn(ConnPool<ConnPtr>* pool) : pool(pool) {
        conn = pool->get();
    }

    ~PoolConn() {
        pool->put(conn);
    }

    ConnPtr& get() {
        return conn;
    }

    ConnPtr& operator->() {
        return conn;
    }
};

#endif // _GRANDET_CONNPOOL_H_
