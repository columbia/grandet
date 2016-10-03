#ifndef _GRANDET_UNIXCONNMGR_H_
#define _GRANDET_UNIXCONNMGR_H_

#include "connmgr.h"
#include <string>

class UNIXConnMgr : public ConnMgr {
    boost::asio::io_service io_service;
    std::unique_ptr<boost::asio::local::stream_protocol::acceptor> acceptor;
    std::string address;

public:
    virtual void start() override;

    UNIXConnMgr(Controller *master, const std::string& address);
    virtual ~UNIXConnMgr();
};

#endif // _GRANDET_UNIXCONNMGR_H_

