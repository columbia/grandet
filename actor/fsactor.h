#ifndef _GRANDET_FSACTOR_H_
#define _GRANDET_FSACTOR_H_

#include "actor.h"

class FSActor : public Actor {
    int ssdDir, magDir;
    bool directMode;

    int getRootDir(const Profile& profile);

public:
    void put(Key& key, std::shared_ptr<Value> value, const Profile& profile, const std::map<std::string, std::string>* metadata = nullptr) override;
    std::shared_ptr<Value> get(Key& key, const Profile& profile, Range range = {}, std::map<std::string, std::string>* metadata = nullptr) override;
    std::shared_ptr<Value> get_url(Key& key, time_t expiration, const Profile& profile) override;
    void del(Key& key, const Profile& profile) override;
    int64_t size(const Key& key, const Profile& profile) override;
    std::string getName() override;

    FSActor();
    ~FSActor();
};

#endif // _GRANDET_FSACTOR_H_

