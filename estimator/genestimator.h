#ifndef _GRANDET_GENESTIMATOR_H_
#define _GRANDET_GENESTIMATOR_H_

#include "estimator.h"
#include <queue>
#include <map>
#include <string>
#include <mutex>
#include <sys/time.h>

class Key;
struct Requirements;

class GenEstimator : public Estimator {
    enum RequestType {
        PUT = 1,
        GET
    };

    struct Bucket {
        std::map<std::string, int> getStat, putStat;
        std::map<std::string, std::map<int, int>> getCounts, putCounts;
        time_t timestamp;
        Bucket(time_t timestamp) : timestamp(timestamp) {}
    };

    std::queue<Bucket> records;
    std::map<std::string, int> getStat, putStat;
    std::map<std::string, std::map<int, int>> getCounts, putCounts;
    std::mutex lock;

    virtual std::string getClue(const Key& key, const Requirements& requirements) = 0;
    void notify(const Key& key, const Requirements& requirements,
            RequestType type);
    double estimate(const Key& key, const Requirements& requirements,
            int interval, RequestType type);

    void purgeOld();
    time_t now();
    int hash(const Key& key);
public:
    virtual double estimateGet(const Key& key, const Requirements& requirements,
            int interval) override;
    virtual double estimatePut(const Key& key, const Requirements& requirements,
            int interval) override;

    virtual void notifyPut(const Key& key, const Requirements& requirements) override;
    virtual void notifyGet(const Key& key, const Requirements& requirements) override;
    virtual void reset() override;
};

#endif // _GRANDET_GENESTIMATOR_H_
