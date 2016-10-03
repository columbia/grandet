#ifndef _GRANDET_TCPCONNMGR_H_
#define _GRANDET_TCPCONNMGR_H_

#include "connmgr.h"

class TCPConnMgr : public ConnMgr {
    boost::asio::io_service io_service;
    std::unique_ptr<boost::asio::ip::tcp::acceptor> acceptor;
    int port;

public:
    virtual void start() override;

    TCPConnMgr(Controller *master, int port);
    virtual ~TCPConnMgr();
};

#endif // _GRANDET_TCPCONNMGR_H_
