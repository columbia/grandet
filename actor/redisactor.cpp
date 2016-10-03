#include <redis3m/simple_pool.h>
#include <redis3m/command.h>

#include "redisactor.h"
#include "actor.h"
#include "key.h"
#include "value.h"
#include "profile.h"
#include "log.h"
#include "config.h"

using namespace redis3m;
using namespace std;

const int DEFAULT_REDIS_PORT = 6379;

class RedisNoSSDPoolException : public std::exception {
    virtual const char* what() const noexcept override {
        return "No SSD pool configured in Redis actor";
    }
};

void RedisActor::put(Key& key, shared_ptr<Value> value, const Profile& profile, const map<string, string>* metadata) {
    simple_pool::ptr_t pool;
    string useSSD;
    if (profile.get("ssd", &useSSD) && useSSD == "true") {
        if (!ssdPool) {
            throw RedisNoSSDPoolException();
        }
        pool = ssdPool;
    } else {
        pool = magPool;
    }
    connection::ptr_t conn = pool->get();
    conn->run(command("SET")(key.toString())(value->toString()));
    if (metadata) {
        for (const auto& meta : *metadata) {
            conn->run(command("HSET")("m:" + key.toString())(meta.first)(meta.second));
        }
    }
    pool->put(conn);
}

shared_ptr<Value> RedisActor::get(Key& key, const Profile& profile, Range range, map<string, string>* metadata) {
    simple_pool::ptr_t pool;
    string useSSD;
    if (profile.get("ssd", &useSSD) && useSSD == "true") {
        if (!ssdPool) {
            throw RedisNoSSDPoolException();
        }
        pool = ssdPool;
    } else {
        pool = magPool;
    }
    connection::ptr_t conn = pool->get();
    Value* result;
    if (range) {
        result = CreateStringValue(conn->run(command("GETRANGE")(key.toString())(range.start.value_or(0))(range.end.value_or(-1))).str());
    } else {
        result = CreateStringValue(conn->run(command("GET")(key.toString())).str());
    }
    if (metadata) {
        const vector<reply> meta = conn->run(command("HGETALL")("m:" + key.toString())).elements();
        for (auto it = meta.begin(); it != meta.end(); ++it) {
            metadata->emplace(it->str(), (++it)->str());
        }
    }
    pool->put(conn);
    return shared_ptr<Value>(result);
}

shared_ptr<Value> RedisActor::get_url(Key& key, time_t expiration, const Profile& profile) {
    return nullptr;
}

void RedisActor::del(Key& key, const Profile& profile) {
    Dbg() << "Redis: deleting " << key.toString() << endl;
    simple_pool::ptr_t pool;
    string useSSD;
    if (profile.get("ssd", &useSSD) && useSSD == "true") {
        if (!ssdPool) {
            throw RedisNoSSDPoolException();
        }
        pool = ssdPool;
    } else {
        pool = magPool;
    }
    connection::ptr_t conn = pool->get();
    conn->run(command("DEL")(key.toString()));
    conn->run(command("DEL")("m:" + key.toString()));
    pool->put(conn);
}

int64_t RedisActor::size(const Key& key, const Profile& profile) {
    simple_pool::ptr_t pool;
    string useSSD;
    if (profile.get("ssd", &useSSD) && useSSD == "true") {
        if (!ssdPool) {
            throw RedisNoSSDPoolException();
        }
        pool = ssdPool;
    } else {
        pool = magPool;
    }
    connection::ptr_t conn = pool->get();
    int64_t result = conn->run(command("STRLEN")(key.toString())).integer();
    pool->put(conn);
    return result;
}

string RedisActor::getName() {
    return "Redis";
}

RedisActor::RedisActor() {
    Config *config = GetConfigInstance();
    int magPort =
            config->get<int>("server.actors.Redis.magPort", DEFAULT_REDIS_PORT);
    int ssdPort =
            config->get<int>("server.actors.Redis.ssdPort", 0);
    magPool = simple_pool::create("127.0.0.1", magPort);
    Dbg() << "connected to magnetic Redis at " << magPort << endl;
    if (ssdPort) {
        ssdPool = simple_pool::create("127.0.0.1", ssdPort);
        Dbg() << "connected to SSD Redis at " << ssdPort << endl;
    }
}

RedisActor::~RedisActor() {
}
