#ifndef _CONDITION_H_
#define _CONDITION_H_

struct Condition {
    long size; // in bytes
    double getRequests; // estimated
    double putRequests;
    int timeSpan; // in seconds
    int durabilityRequired;
    int latencyRequired;
    int bandwidthRequired;
};

#endif // _CONDITION_H_
