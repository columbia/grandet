#ifndef _GRANDET_S3ACTOR_H_
#define _GRANDET_S3ACTOR_H_

#include <libaws/aws.h>
#include "actor.h"
#include "connpool.h"

class S3Actor : public Actor {
    ConnPool<aws::S3ConnectionPtr> pool;

public:
    void put(Key& key, std::shared_ptr<Value> value, const Profile& profile, const std::map<std::string, std::string>* metadata = nullptr) override;
    std::shared_ptr<Value> get(Key& key, const Profile& profile, Range range = {}, std::map<std::string, std::string>* metadata = nullptr) override;
    std::shared_ptr<Value> get_url(Key& key, time_t expiration, const Profile& profile) override;
    void del(Key& key, const Profile& profile) override;
    int64_t size(const Key& key, const Profile& profile) override;
    std::string getName() override;

    S3Actor();
};

#endif // _GRANDET_S3ACTOR_H_
