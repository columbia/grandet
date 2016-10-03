#ifndef _GRANDET_ACTOR_H_
#define _GRANDET_ACTOR_H_

#include <map>
#include <memory>
#include <string>
#include <sys/time.h>

#include "range.h"

class Key;
struct Value;
class Profile;

struct Actor {
    // Operations
    virtual void put(Key& key, std::shared_ptr<Value> value, const Profile& profile, const std::map<std::string, std::string>* metadata = nullptr) = 0;
    virtual std::shared_ptr<Value> get(Key& key, const Profile& profile, Range range = {}, std::map<std::string, std::string>* metadata = nullptr) = 0;
    virtual std::shared_ptr<Value> get_url(Key& key, time_t expiration, const Profile& profile) = 0;
    virtual void del(Key& key, const Profile& profile) = 0;
    virtual int64_t size(const Key& key, const Profile& profile) = 0;

    virtual std::string getName() = 0;
    virtual ~Actor() = default;
};

#endif // _GRANDET_ACTOR_H_
