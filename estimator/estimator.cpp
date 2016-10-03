#include "estimator.h"
#include "key.h"
#include "config.h"
#include "timegen.h"
#include <queue>
#include <string>
#include <map>
#include <sys/time.h>

#include "nsestimator.h"
#include "metaestimator.h"

using namespace std;

const int STAT_INTERVAL = 86400;

class BasicEstimator : public Estimator {
    typedef string key_t;
    typedef time_t time_type;

    enum RecordType {
        PUT = 0,
        GET,
    };

    struct Record {
        key_t key;
        RecordType type;
        time_type time;
    };

    queue<Record> records;
    map<key_t, int> getCounts, putCounts;

    void purgeOld();
    void newRecord(RecordType type, key_t key);
    void removeRecord(const Record& record);
public:
    virtual double estimateGet(const Key& key, const Requirements& requirements,
            int interval) override;
    virtual double estimatePut(const Key& key, const Requirements& requirements,
            int interval) override;

    virtual void notifyPut(const Key& key, const Requirements& requirements) override;
    virtual void notifyGet(const Key& key, const Requirements& requirements) override;

    virtual void reset();
    virtual ~BasicEstimator();
};

double BasicEstimator::estimateGet(const Key& key, const Requirements& requirements,
        int interval) {
    return 1.0 * getCounts[key.toString()] * interval / STAT_INTERVAL;
}

double BasicEstimator::estimatePut(const Key& key, const Requirements& requirements,
        int interval) {
    return 1.0 * putCounts[key.toString()] * interval / STAT_INTERVAL;
}

void BasicEstimator::notifyPut(const Key& key, const Requirements& requirements) {
    newRecord(PUT, key.toString());
    purgeOld();
}

void BasicEstimator::notifyGet(const Key& key, const Requirements& requirements) {
    newRecord(GET, key.toString());
    purgeOld();
}

void BasicEstimator::newRecord(RecordType type, key_t key) {
    records.push(Record{key, type});
    if (type == PUT) {
        putCounts[key]++;
    } else {
        getCounts[key]++;
    }
}

void BasicEstimator::removeRecord(const Record& record) {
    if (record.type == PUT) {
        putCounts[record.key]--;
    } else {
        getCounts[record.key]--;
    }
}

void BasicEstimator::purgeOld() {
    time_type cur = TimeGen::now();
    while (!records.empty()) {
        const Record& record = records.front();
        if (record.time < cur - STAT_INTERVAL) {
            removeRecord(record);
            records.pop();
        } else {
            break;
        }
    }
}

void BasicEstimator::reset() {
    records = queue<Record>();
    getCounts.clear();
    putCounts.clear();
}

BasicEstimator::~BasicEstimator() {
}

class UnknownEstimatorException : public exception {
    string name;
public:
    UnknownEstimatorException(const string& name) : name(name) {}
    virtual const char* what() const noexcept override {
        return name.c_str();
    }
};

Estimator* GetEstimator() {
    Config *config = GetConfigInstance();
    string estimatorName = config->get<string>("server.estimator", "basic");
    if (estimatorName == "basic") {
        return new BasicEstimator();
    } else if (estimatorName == "namespace") {
        return new NamespaceEstimator();
    } else if (estimatorName == "meta") {
        return new MetaEstimator();
    } else {
        throw UnknownEstimatorException(estimatorName);
    }
}
