#include "config.h"
#include <memory>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include "log.h"

using namespace std;
using boost::property_tree::ptree;

Config *config = nullptr;

class ConfigImpl {
    unique_ptr<ptree> pt;

public:
    template <class T>
        T get(const string& path) {
            return get<T>(pt.get(), path);
        }
    template <class T>
        T get(ptree* node, const string& path) {
            return getChild(node, path, NULL)->get_value<T>();
        }

    ptree* getChild(ptree* node, const string& path, string* key) {
        size_t pos = path.find(".");
        string curr_part;
        if (pos == string::npos) {
            curr_part = path;
        } else {
            curr_part = path.substr(0, pos);
        }
        try {
            int idx = stoi(curr_part);
            auto it = node->begin();
            for (int i=0; i<idx; i++) it++;
            if (pos == string::npos) {
                if (key) *key = it->first;
                return &it->second;
            } else {
                return getChild(&it->second, path.substr(pos + 1), key);
            }
        } catch (invalid_argument e) {
            ptree* child = &node->get_child(curr_part);
            if (pos == string::npos) {
                if (key) *key = curr_part;
                return child;
            } else {
                return getChild(child, path.substr(pos + 1), key);
            }
        }
    }
    int getSize(const string& path) {
        try {
            ptree* child = getChild(pt.get(), path, NULL);
            return child->size();
        } catch (...) {
            return 0;
        }
    }
    string getName(const string& path) {
        string key;
        getChild(pt.get(), path, &key);
        return key;
    }
    void load(const string& filename) {
        if (!pt) {
            pt = make_unique<ptree>();
        }

        ifstream inf(filename);
        if (inf.good()) {
            inf.close();
            Info() << "loading config from " << filename << endl;
            read_json(filename, *pt);
        } else {
            inf.close();
            Err() << "cannot find config file " << filename << endl;
            abort();
        }
    }
};

Config::Config() {
    impl = new ConfigImpl();
}

Config::~Config() {
    delete impl;
}

void Config::load(const string& filename) {
    impl->load(filename);
}

Config* GetConfigInstance() {
    if (config == nullptr) {
        config = new Config();
    }
    return config;
}

template <class T>
T Config::get(const std::string& path) {
    return impl->get<T>(path);
}

template <class T>
T Config::get(const string& path, const T& defaultValue) {
    try {
        return impl->get<T>(path);
    } catch (...) {
        return defaultValue;
    }
}

int Config::getSize(const string& path) {
    return impl->getSize(path);
}

string Config::getName(const string& path) {
    return impl->getName(path);
}

template int Config::get<int>(const std::string&);
template long Config::get<long>(const std::string&);
template double Config::get<double>(const std::string&);
template string Config::get<string>(const std::string&);
template bool Config::get<bool>(const std::string&);

template int Config::get<int>(const std::string&, const int& defaultValue);
template long Config::get<long>(const std::string&, const long& defaultValue);
template double Config::get<double>(const std::string&, const double& defaultValue);
template string Config::get<string>(const std::string&, const string& defaultValue);
template bool Config::get<bool>(const std::string&, const bool& defaultValue);
