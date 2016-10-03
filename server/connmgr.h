#ifndef _GRANDET_CONNMGR_H_
#define _GRANDET_CONNMGR_H_

#include <boost/asio.hpp>
#include "boost/threadpool.hpp"
#include <memory>
#include "network.h"

namespace grandet {
    class Request;
}
class Controller;
struct Requirements;
class ConnInfo;
class Key;

class ConnMgr {
    std::unique_ptr<boost::threadpool::pool> pool;

    // #return false: error
    bool doUnlock(const Key& key);

    // Handle different operations
    // #return false: connection should be dropped
    bool handleGet(const grandet::Request& req, ConnInfo* conn);
    bool handlePut(grandet::Request& req, ConnInfo* conn);
    bool handleDel(const grandet::Request& req, ConnInfo* conn);
    bool handleSize(const grandet::Request& req, ConnInfo* conn);
    bool handleKeys(const grandet::Request& req, ConnInfo* conn);
    bool handleDist(const grandet::Request& req, ConnInfo* conn);
    bool handleLock(const grandet::Request& req, ConnInfo* conn,
            bool* shouldSuspend);
    bool handleUnlock(const grandet::Request& req, ConnInfo* conn);
    bool handleReset(const grandet::Request& req, ConnInfo* conn);
    bool handleAccount(const grandet::Request& req, ConnInfo* conn);

    bool sendResponse(bool ok, flow_t* stream);

    void handleClient(ConnInfo* conn, bool first);
    void readRequirements(const grandet::Request &req, Requirements *requirements);

protected:
    Controller *master;

    void scheduleClient(ConnInfo *conn, bool first);

public:
    virtual void start() = 0;

    ConnMgr();
    virtual ~ConnMgr();
};

#endif // _GRANDET_CONNMGR_H_
