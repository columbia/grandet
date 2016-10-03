#include "conninfo.h"
#include "tcpconnmgr.h"
#include "log.h"

#include <boost/asio.hpp>
#include <memory>
using namespace std;
using boost::asio::ip::tcp;

class TCPConnInfo : public ConnInfo {
    std::unique_ptr<tcp::iostream> stream;

public:
    TCPConnInfo() {
        stream = make_unique<tcp::iostream>();
    }

    virtual tcp::iostream* getStream() {
        return stream.get();
    }

    virtual ~TCPConnInfo() {
        stream->close();
    }
};

void TCPConnMgr::start() {
    acceptor = make_unique<tcp::acceptor>(io_service, tcp::endpoint(tcp::v4(), port));
    acceptor->set_option(tcp::no_delay(true));
    Info() << "listening at " << port << endl;

    while (true) {
        Dbg() << "wait client" << endl;
        TCPConnInfo *conn = new TCPConnInfo();
        acceptor->accept(*conn->getStream()->rdbuf());

        Dbg() << "got client" << endl;
        scheduleClient(conn, true);
    }
}

TCPConnMgr::TCPConnMgr(Controller *master, int port) {
    this->master = master;
    this->port = port;
}

TCPConnMgr::~TCPConnMgr() {
}
