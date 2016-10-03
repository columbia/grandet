#include <redis3m/simple_pool.h>
#include <redis3m/command.h>

#include "memory.h"
#include "config.h"
#include "log.h"
using namespace std;
using namespace redis3m;

const string prefix = "s:m:";

Memory::choice_id Memory::remember(const Key& key, const choice_id& choice) {
    lock_guard<mutex> guard(lock);
    choice_id old_choice = 0;
    auto it = data.find(key);
    if (it != data.end()) {
        if (it->second != choice) {
            old_choice = it->second;
        }
    }
    data[key] = choice;
    if (persist) {
        connection::ptr_t conn = pool->get();
        conn->run(command("SET")(prefix + key.toString())(choice));
        pool->put(conn);
    }
    return old_choice;
}

bool Memory::recall(const Key& key, choice_id* choice) {
    lock_guard<mutex> guard(lock);
    auto it = data.find(key);
    if (it != data.end()) {
        *choice = it->second;
        return true;
    } else {
        return false;
    }
}

void Memory::forget(const Key& key) {
    lock_guard<mutex> guard(lock);
    data.erase(key);
    if (persist) {
        connection::ptr_t conn = pool->get();
        conn->run(command("DEL")(prefix + key.toString()));
        pool->put(conn);
    }
}

void Memory::load() {
    lock_guard<mutex> guard(lock);
    if (persist) {
        pool = simple_pool::create("127.0.0.1");
        connection::ptr_t conn;
        try {
            conn = pool->get();
        } catch (...) {
            Err() << "fail to connect to redis to retrieve memory" << endl;
            abort();
        }
        const reply& rep = conn->run(command("KEYS")(prefix + "*"));
        assert(rep.type() == reply::ARRAY);
        for (const auto& item : rep.elements()) {
            const string& key = item.str().substr(prefix.size());
            choice_id choice;
            const reply& val = conn->run(command("GET")(item.str()));
            if (val.type() == reply::STRING) {
                choice = stoi(val.str());
            } else if (val.type() == reply::INTEGER) {
                choice = val.integer();
            } else {
                Warn() << "fail to get memory of key " << key <<
                    ", type = " << val.type() << endl;
                continue;
            }

            data.insert(make_pair(Key(key), choice));
        }
        pool->put(conn);
    }
}

Memory::Memory() {
    lock_guard<mutex> guard(lock);
    persist = bool(GetConfigInstance()->get<int>("server.persist", 1));
    Info() << "Memory will " << (persist ? "" : "not ") << "be persisted." << endl;
}

const map<Key, Memory::choice_id>& Memory::dump() {
    return data;
}

void Memory::lockIt() {
    lock.lock();
}

void Memory::unlockIt() {
    lock.unlock();
}
