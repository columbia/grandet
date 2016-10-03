#ifndef _GRANDET_CONFIG_H_
#define _GRANDET_CONFIG_H_

#include <string>

class ConfigImpl;

class Config {
    ConfigImpl *impl;
public:
    void load(const std::string& filename);
    template <class T>
    T get(const std::string& path);
    template <class T>
    T get(const std::string& path, const T& defaultValue);
    int getSize(const std::string& path);
    std::string getName(const std::string& path);

    Config();
    ~Config();
};

Config* GetConfigInstance();

#endif // _GRANDET_CONFIG_H_
