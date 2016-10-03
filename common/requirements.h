#ifndef _GRANDET_REQUIREMENTS_H_
#define _GRANDET_REQUIREMENTS_H_

#include <unordered_map>

#include "range.h"

struct Requirements {
    int durabilityRequired;
    int latencyRequired;
    int bandwidthRequired;
    std::map<std::string, std::string> metadata;
    bool prefer_url;
    time_t expiration;
    Range range;
};

#endif // _GRANDET_REQUIREMENTS_H_
