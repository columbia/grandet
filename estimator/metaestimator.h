#ifndef _GRANDET_METAESTIMATOR_H_
#define _GRANDET_METAESTIMATOR_H_

#include <map>
#include <mutex>
#include "genestimator.h"

class MetaEstimator : public GenEstimator {
    std::mutex clueLock;
    std::map<std::string, std::string> clueStore;
    virtual std::string getClue(const Key& key, const Requirements& requirements) override;
    bool ignore;

public:
    MetaEstimator();
};

#endif // _GRANDET_METAESTIMATOR_H_
