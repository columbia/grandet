#ifndef _GRANDET_DUMMYACTOR_H_
#define _GRANDET_DUMMYACTOR_H_

#include <mutex>
#include <map>
#include <string>
#include "actor.h"

class DummyActor : public Actor {
    struct Bucket {
        std::mutex lock;
        std::map<std::string, std::string> data;
        std::map<std::string, std::map<std::string, std::string>> meta;
    };

    std::map<std::string, Bucket> data;
    std::mutex lock;

    Bucket* getBucket(const Profile& profile);

public:
    void put(Key& key, std::shared_ptr<Value> value, const Profile& profile, const std::map<std::string, std::string>* metadata = nullptr) override;
    std::shared_ptr<Value> get(Key& key, const Profile& profile, Range range = {}, std::map<std::string, std::string>* metadata = nullptr) override;
    std::shared_ptr<Value> get_url(Key& key, time_t expiration, const Profile& profile) override;
    void del(Key& key, const Profile& profile) override;
    int64_t size(const Key& key, const Profile& profile) override;
    std::string getName() override;
};

#endif // _GRANDET_DUMMYACTOR_H_

