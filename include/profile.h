#ifndef _GRANDET_PROFILE_H_
#define _GRANDET_PROFILE_H_

#include <map>
#include <string>

class Config;

class Profile {
    std::map<std::string, std::string> data;

public:
    bool get(const std::string& key, std::string* value) const;
    void put(const std::string& key, const std::string& value);
    void load(Config *config, const std::string& path);

    std::string desc() const;
};

#endif // _GRANDET_PROFILE_H_
