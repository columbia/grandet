#ifndef __GRANDET_SDK_CPP_GRANDET_H_
#define __GRANDET_SDK_CPP_GRANDET_H_

#include <string>
#include "statusor.h"

class GrandetClient {
public:
    virtual void connect(const std::string& server, const std::string& port) = 0;
    virtual void connect_unix(const std::string& server) = 0;
    virtual ErrorCode put(const std::string& key, const std::string& value,
            int latency_required = 100, int bandwidth_required = 1) = 0;

    virtual StatusOr<std::string> get(const std::string& key, int latency_required = 100,
            int bandwidth_required = 1, int range_start = 0, int range_end = 0) = 0;

    virtual ~GrandetClient() {}
};

GrandetClient* CreateGrandetClient();

#endif // __GRANDET_SDK_CPP_GRANDET_H_

