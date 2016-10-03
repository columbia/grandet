#ifndef _GRANDET_NSESTIMATOR_H_
#define _GRANDET_NSESTIMATOR_H_

#include "genestimator.h"
#include <queue>
#include <map>
#include <string>
#include <sys/time.h>

class Key;
struct Requirements;

class NamespaceEstimator : public GenEstimator {
    virtual std::string getClue(const Key& key, const Requirements& requirements) override;
};

#endif // _GRANDET_NSESTIMATOR_H_
