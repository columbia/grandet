#include "genestimator.h"
#include "key.h"
#include "log.h"
#include "timegen.h"

using namespace std;

const int RECORD_INTERVAL = 3600 * 24 * 30;
const int BUCKET_INTERVAL = 60;
const int GROUP_SIZE = 65537;

double GenEstimator::estimateGet(const Key& key, const Requirements& requirements,
            int interval) {
    return estimate(key, requirements, interval, GET);
}

double GenEstimator::estimatePut(const Key& key, const Requirements& requirements,
            int interval) {
    return estimate(key, requirements, interval, PUT);
}

double GenEstimator::estimate(const Key& key, const Requirements& requirements,
        int interval, RequestType type) {
    lock_guard<mutex> guard(lock);
    const string& clue = getClue(key, requirements);
    int keyHash = hash(key);
    int count;
    if (type == GET) {
        count = getCounts[clue].size();
    } else {
        count = putCounts[clue].size();
    }
    if (count == 0) count = 1;
    int reqCount;
    if (type == GET) {
        if (getCounts[clue].count(keyHash) == 0) {
            reqCount = 0;
        } else {
            reqCount = getStat[clue];
        }
    } else {
        reqCount = putStat[clue];
    }
    double ret = 1.0 * reqCount * interval / RECORD_INTERVAL / count;
    Dbg() << "estimate: " << clue << " cnt: " << reqCount
        << " card: " << count << " ret: " << ret << endl;
    return ret;
}

void GenEstimator::notifyPut(const Key& key, const Requirements& requirements) {
    notify(key, requirements, PUT);
    purgeOld();
}
void GenEstimator::notifyGet(const Key& key, const Requirements& requirements) {
    notify(key, requirements, GET);
    purgeOld();
}

void GenEstimator::purgeOld() {
    lock_guard<mutex> guard(lock);
    while (!records.empty() && records.front().timestamp < TimeGen::now() - RECORD_INTERVAL) {
        for (const auto& it : records.front().getStat) {
            getStat[it.first] -= it.second;
            for (const auto& acount : records.front().getCounts[it.first]) {
                getCounts[it.first][acount.first] -= acount.second;
                if (getCounts[it.first][acount.first] == 0) {
                    getCounts[it.first].erase(acount.first);
                }
            }
        }
        for (const auto& it : records.front().putStat) {
            putStat[it.first] -= it.second;
            for (const auto& acount : records.front().putCounts[it.first]) {
                putCounts[it.first][acount.first] -= acount.second;
                if (putCounts[it.first][acount.first] == 0) {
                    putCounts[it.first].erase(acount.first);
                }
            }
        }
        records.pop();
    }
}

void GenEstimator::notify(const Key& key, const Requirements& requirements,
        RequestType type) {
    lock_guard<mutex> guard(lock);
    if (records.empty() || records.back().timestamp < TimeGen::now() - BUCKET_INTERVAL) {
        records.push(Bucket(TimeGen::now()));
    }
    string clue = getClue(key, requirements);
    Bucket& record = records.back();
    int keyHash = hash(key);
    if (type == PUT) {
        record.putStat[clue]++;
        putStat[clue]++;

        record.putCounts[clue][keyHash]++;
        putCounts[clue][keyHash]++;
    } else {
        record.getStat[clue]++;
        getStat[clue]++;

        record.getCounts[clue][keyHash]++;
        getCounts[clue][keyHash]++;
    }
}

int GenEstimator::hash(const Key& key) {
    return std::hash<string>()(key.toString()) % GROUP_SIZE;
}

void GenEstimator::reset() {
    lock_guard<mutex> guard(lock);

    records = queue<Bucket>();
    getStat.clear();
    putStat.clear();
    getCounts.clear();
    putCounts.clear();
}
