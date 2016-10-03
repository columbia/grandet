#ifndef _GRANDET_REDISACTOR_H_
#define _GRANDET_REDISACTOR_H_

#include <redis3m/simple_pool.h>
#include "actor.h"

class RedisActor : public Actor {
    redis3m::simple_pool::ptr_t magPool;
    redis3m::simple_pool::ptr_t ssdPool;

public:
    void put(Key& key, std::shared_ptr<Value> value, const Profile& profile, const std::map<std::string, std::string>* metadata = nullptr) override;
    std::shared_ptr<Value> get(Key& key, const Profile& profile, Range range = {}, std::map<std::string, std::string>* metadata = nullptr) override;
    std::shared_ptr<Value> get_url(Key& key, time_t expiration, const Profile& profile) override;
    void del(Key& key, const Profile& profile) override;
    int64_t size(const Key& key, const Profile& profile) override;
    std::string getName() override;

    RedisActor();
    ~RedisActor();
};

#endif // _GRANDET_REDISACTOR_H_
