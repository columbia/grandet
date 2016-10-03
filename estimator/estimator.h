#ifndef _GRANDET_ESTIMATOR_
#define _GRANDET_ESTIMATOR_

class Key;
struct Requirements;

struct Estimator {
    virtual double estimateGet(const Key& key, const Requirements& requirements,
            int interval) = 0;
    virtual double estimatePut(const Key& key, const Requirements& requirements,
            int interval) = 0;

    virtual void notifyPut(const Key& key, const Requirements& requirements) = 0;
    virtual void notifyGet(const Key& key, const Requirements& requirements) = 0;

    virtual void reset() = 0;
    virtual ~Estimator() = default;
};

Estimator* GetEstimator();

#endif // _GRANDET_ESTIMATOR_
