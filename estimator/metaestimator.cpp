#include "metaestimator.h"
#include "requirements.h"
#include "log.h"
#include "key.h"
#include "config.h"

#include <sstream>
using namespace std;

string MetaEstimator::getClue(const Key& key, const Requirements& requirements) {
    if (ignore) return "";
    stringstream ret;
    bool first = true;
    for (const auto& pair : requirements.metadata) {
        if (!first) {
            ret << "/";
        }
        first = false;
        ret << pair.first << ":" << pair.second;
    }

    lock_guard<mutex> guard(clueLock);
    if (first) {
        const auto &it = clueStore.find(key.toString());
        if (it != clueStore.end()) {
            Dbg() << "meta: " << it->second << endl;
            return it->second;
        }
    } else {
        const auto &it = clueStore.find(key.toString());
        if (it == clueStore.end()) {
            clueStore[key.toString()] = ret.str();
        }
        Dbg() << "meta: " << ret.str() << endl;
    }
    return ret.str();
}

MetaEstimator::MetaEstimator() {
    ignore = GetConfigInstance()->get<bool>("server.estimators.meta.ignore", false);
    if (ignore) {
        Info() << "Ignoring Metadata in MetaEstimator" << endl;
    }
}
