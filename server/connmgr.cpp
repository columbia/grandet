#include "connmgr.h"
#include "grandet.pb.h"
#include "log.h"
#include "key.h"
#include "value.h"
#include "controller.h"
#include "requirements.h"
#include "network.h"
#include "config.h"
#include "conninfo.h"

using boost::asio::ip::tcp;
using namespace std;

void ConnMgr::scheduleClient(ConnInfo* conn, bool first) {
    pool->schedule(boost::bind(&ConnMgr::handleClient, this, conn, first));
}

void ConnMgr::handleClient(ConnInfo* conn, bool first) {
    if (first) {
        Dbg() << "handling client" << endl;
    } else {
        Dbg() << "continue client" << endl;
    }
    while (true) {
        grandet::Request req;
        if (!ReadMessage(*conn->getStream(), req)) {
            break;
        }

        bool ok = true;
        switch (req.type()) {
            case grandet::Request::PUT:
                ok = handlePut(req, conn);
                break;
            case grandet::Request::GET:
                ok = handleGet(req, conn);
                break;
            case grandet::Request::DEL:
                ok = handleDel(req, conn);
                break;
            case grandet::Request::SIZE:
                ok = handleSize(req, conn);
                break;
            case grandet::Request::KEYS:
                ok = handleKeys(req, conn);
                break;
            case grandet::Request::LOCK: {
                bool shouldSuspend;
                ok = handleLock(req, conn, &shouldSuspend);
                if (shouldSuspend) {
                    // we don't want to close the connection
                    // just suspend it
                    return;
                }
                break;
            }
            case grandet::Request::UNLOCK:
                ok = handleUnlock(req, conn);
                break;
            case grandet::Request::RESET:
                ok = handleReset(req, conn);
                break;
            case grandet::Request::DIST:
                ok = handleDist(req, conn);
                break;
            case grandet::Request::ACCOUNT:
                ok = handleAccount(req, conn);
                break;
            case grandet::Request::EXIT:
                exit(0);
            default:
                Warn() << "Unknown request type: " << req.type() << endl;
                sendResponse(false, conn->getStream());
        }
        if (!ok) break;
    }
    Dbg() << "client finished" << endl;
    for (const auto& key : conn->getLockedKeys()) {
        doUnlock(key);
    }
    delete conn;
}

ConnMgr::ConnMgr() {
    pool = make_unique<boost::threadpool::pool>(GetConfigInstance()->get<int>("server.conns", 10));
}

void ConnMgr::readRequirements(const grandet::Request &req, Requirements *requirements) {
    requirements->durabilityRequired = req.requirements().durability_required();
    requirements->latencyRequired = req.requirements().latency_required();
    requirements->bandwidthRequired = req.requirements().bandwidth_required();
    auto& proto_metadata = req.requirements().metadata();
    auto& metadata = requirements->metadata;
    for (const auto& meta : proto_metadata) {
        metadata.emplace(meta.meta_key(), meta.meta_value());
    }
    requirements->prefer_url = req.requirements().prefer_url();
    requirements->expiration = req.requirements().expiration();
    if (req.requirements().has_range()) {
        auto& proto_range = req.requirements().range();
        Range& range = requirements->range;
        if (proto_range.has_start()) {
            range.start = proto_range.start();
        }
        if (proto_range.has_end()) {
            range.end = proto_range.end();
        }
    }
}

ConnMgr::~ConnMgr() {
}

bool ConnMgr::handlePut(grandet::Request& req, ConnInfo* conn) {
    Key key(req.key());
    shared_ptr<Value> value;
    switch (req.value().type()) {
        case grandet::Value::DATA:
            value = shared_ptr<Value>(LootStringValue(*req.mutable_value()->mutable_data()));
            break;
        case grandet::Value::FILENAME:
            value = shared_ptr<Value>(CreateFileValue(req.value().filename()));
            break;
        default:
            Err() << "Unknown value type: " << req.value().type() << endl;
            return false;
    }
    Requirements requirements;
    readRequirements(req, &requirements);

    Dbg() << "REQUEST: PUT " << req.key() << endl;
    for (const auto& meta : requirements.metadata) {
        Dbg() << "Metadata: " << meta.first << " => " << meta.second << endl;
    }

    bool error = false;
    try {
        master->handlePut(key, value, requirements);
    } catch (const exception& e) {
        error = true;
        Err() << e.what() << endl;
    }

    sendResponse(!error, conn->getStream());
    if (error) {
        Warn() << "FAIL:    PUT " << req.key() << endl;
    } else {
        Dbg() << "DONE:    PUT " << req.key() << endl;
    }

    return true;
}

bool ConnMgr::handleGet(const grandet::Request& req, ConnInfo* conn) {
    Key key(req.key());
    Requirements requirements;
    readRequirements(req, &requirements);

    Dbg() << "REQUEST: GET " << req.key() << endl;

    shared_ptr<Value> value;
    bool error = false;
    try {
        value = master->handleGet(key, requirements);
    } catch (const exception& e) {
        error = true;
        Err() << e.what() << endl;
    }

    grandet::Response resp;
    if (!error && value) {
        resp.set_status(grandet::Response::OK);
        grandet::Value* resp_value = resp.mutable_value();
        if (value->hasUrl()) {
            resp_value->set_type(grandet::Value::URL);
            resp_value->set_url(value->toUrl());
        } else {
            resp_value->set_type(grandet::Value::DATA);
            resp_value->set_data(value->toString());
        }
        Dbg() << "DONE:    GET " << req.key() << endl;
    } else {
        resp.set_status(grandet::Response::ERROR);
        Warn() << "FAIL:    GET " << req.key() << endl;
    }
    if (!WriteMessage(*conn->getStream(), resp)) {
        Err() << "Fail to send response" << endl;
    }

    return true;
}

bool ConnMgr::handleDel(const grandet::Request& req, ConnInfo* conn) {
    Key key(req.key());
    Requirements requirements;
    readRequirements(req, &requirements);

    Dbg() << "REQUEST: DEL " << req.key() << endl;

    bool error = false;
    try {
        master->handleDel(key, requirements);
    } catch (const exception& e) {
        error = true;
        Err() << e.what() << endl;
    }

    sendResponse(!error, conn->getStream());
    if (!error) {
        Dbg() << "DONE:    DEL " << req.key() << endl;
    } else {
        Warn() << "FAIL:    DEL " << req.key() << endl;
    }

    return true;
}

bool ConnMgr::handleSize(const grandet::Request& req, ConnInfo* conn) {
    Key key(req.key());
    Requirements requirements;
    readRequirements(req, &requirements);

    Dbg() << "REQUEST: SIZE " << req.key() << endl;

    int64_t size;
    bool error = false;
    try {
        size = master->handleSize(key, requirements);
    } catch (const exception& e) {
        error = true;
        Err() << e.what() << endl;
    }

    grandet::Response resp;
    if (!error && size >= 0) {
        resp.set_status(grandet::Response::OK);
        grandet::Value* resp_value = resp.mutable_value();
        resp_value->set_type(grandet::Value::SIZE);
        resp_value->set_size(size);
        Dbg() << "DONE:    SIZE " << req.key() << endl;
    } else {
        resp.set_status(grandet::Response::ERROR);
        Warn() << "FAIL:    SIZE " << req.key() << endl;
    }
    if (!WriteMessage(*conn->getStream(), resp)) {
        Err() << "Fail to send response" << endl;
    }

    return true;
}

bool ConnMgr::handleKeys(const grandet::Request& req, ConnInfo* conn) {
    grandet::Response resp;
    resp.set_status(grandet::Response::OK);
    resp.mutable_value()->set_type(grandet::Value::DATA);
    resp.mutable_value()->set_data(master->handleKeys());

    WriteMessage(*conn->getStream(), resp);
    return true;
}

bool ConnMgr::handleLock(const grandet::Request& req, ConnInfo* conn,
        bool *shouldSuspend) {
    Key key(req.key());

    Dbg() << "REQUEST: LCK " << req.key() << endl;

    if (conn->locked(key)) {
        Warn() << "already locked " << req.key() << endl;
        // should we return an error?
        sendResponse(true, conn->getStream());
        return true;
    }

    bool error = false;
    try {
        bool succ = master->handleLock(key, conn);
        if (!succ) {
            *shouldSuspend = true;
        } else {
            *shouldSuspend = false;
            conn->addLocked(key);
        }
    } catch (const exception& e) {
        error = true;
        Err() << e.what() << endl;
    }

    // if lock needs wait, suspend
    if (*shouldSuspend) {
        Dbg() << "WAIT:    LCK " << req.key() << endl;
        return true;
    }
    // lock succeeded
    sendResponse(!error, conn->getStream());
    if (error) {
        Warn() << "FAIL:    LCK " << req.key() << endl;
    } else {
        Dbg() << "DONE:    LCK " << req.key() << endl;
    }

    return true;
}

bool ConnMgr::doUnlock(const Key& key) {
    try {
        ConnInfo *wake = master->handleUnlock(key);
        if (wake) {
            // wake up another connection
            wake->addLocked(key);
            sendResponse(true, wake->getStream());
            scheduleClient(wake, false);
        }
    } catch (const exception& e) {
        return true;
        Err() << e.what() << endl;
    }
    return false;
}

bool ConnMgr::handleUnlock(const grandet::Request& req, ConnInfo* conn) {
    Key key(req.key());

    Dbg() << "REQUEST: ULK " << req.key() << endl;

    if (!conn->locked(key)) {
        Warn() << "not locked: " << req.key() << endl;
        sendResponse(true, conn->getStream());
        return true;
    }

    bool error = doUnlock(key);

    sendResponse(!error, conn->getStream());
    if (error) {
        Warn() << "FAIL:    ULK " << req.key() << endl;
    } else {
        conn->removeLocked(key);
        Dbg() << "DONE:    ULK " << req.key() << endl;
    }
    return true;
}

bool ConnMgr::handleReset(const grandet::Request& req, ConnInfo* conn) {
    master->handleReset();
    sendResponse(true, conn->getStream());
    return true;
}

bool ConnMgr::handleDist(const grandet::Request& req, ConnInfo* conn) {
    grandet::Response resp;
    resp.set_status(grandet::Response::OK);
    resp.mutable_value()->set_type(grandet::Value::DATA);
    resp.mutable_value()->set_data(master->handleDist());

    WriteMessage(*conn->getStream(), resp);
    return true;
}

bool ConnMgr::handleAccount(const grandet::Request& req, ConnInfo* conn) {
    master->handleAccount();
    sendResponse(true, conn->getStream());
    return true;
}

bool ConnMgr::sendResponse(bool ok, flow_t *stream) {
    grandet::Response resp;
    if (ok) {
        resp.set_status(grandet::Response::OK);
    } else {
        resp.set_status(grandet::Response::ERROR);
    }
    bool ret = WriteMessage(*stream, resp);
    if (!ret) {
        Err() << "Fail to send response" << endl;
    }
    return ret;
}
