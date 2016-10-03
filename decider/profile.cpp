#include <sstream>

#include "profile.h"
#include "config.h"
#include "log.h"
using namespace std;

bool Profile::get(const string& key, string* value) const {
    auto it = data.find(key);
    if (it == data.end()) {
        return false;
    }

    *value = it->second;

    return true;
}

void Profile::put(const string& key, const string& value) {
    data.insert(make_pair(key, value));
}

void Profile::load(Config *config, const std::string& path) {
    for (int i=0; i<config->getSize(path + "profile"); i++) {
        string key = config->getName(path + "profile." + to_string(i));
        string value = config->get<string>(path + "profile." + to_string(i));
        put(key, value);
        Dbg() << "profile: " << key << " = " << value << endl;
    }
}

string Profile::desc() const {
    stringstream ss;
    bool first = true;
    for (const auto& it : data) {
        if (!first) ss << ",";
        first = false;
        ss << it.first << "=" << it.second;
    }
    return ss.str();
}
