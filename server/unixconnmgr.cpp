#include "conninfo.h"
#include "unixconnmgr.h"
#include "log.h"

#if USE_ASIO
#include <boost/asio.hpp>
#include <memory>
using boost::asio::local::stream_protocol;
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#endif

using namespace std;

class UNIXConnInfo : public ConnInfo {
#if USE_ASIO
    std::unique_ptr<stream_protocol::iostream> stream;
#else
    int sock;
#endif

public:
    UNIXConnInfo() {
#if USE_ASIO
        stream = make_unique<stream_protocol::iostream>();
#endif
    }

    virtual flow_t* getStream() {
#if USE_ASIO
        return stream.get();
#else
        return &sock;
#endif
    }

    virtual ~UNIXConnInfo() {
#if USE_ASIO
        stream->close();
#else
        close(sock);
#endif
    }
};

void UNIXConnMgr::start() {
    unlink(address.c_str());
#if USE_ASIO
    stream_protocol::endpoint ep(address);
    acceptor = make_unique<stream_protocol::acceptor>(io_service, ep);
    stream_protocol::socket socket(io_service);
#else
    int sock;
    sock = socket(AF_UNIX, SOCK_STREAM, 0);
    struct sockaddr_un server;
    server.sun_family = AF_UNIX;
    strcpy(server.sun_path, address.c_str());
    if (bind(sock, (struct sockaddr *)&server, sizeof(struct sockaddr_un)) != 0) {
        Err() << "fail to bind UNIX socket: " << strerror(errno) << endl;
        return;
    }
    listen(sock, 10);
#endif
    Info() << "listening at " << address << endl;

    while (true) {
        Dbg() << "wait client" << endl;
        UNIXConnInfo *conn = new UNIXConnInfo();
#if USE_ASIO
        acceptor->accept(*conn->getStream()->rdbuf());
#else
        int client_sock = accept(sock, 0, 0);
        if (client_sock == -1) break;
        *conn->getStream() = client_sock;
#endif

        Dbg() << "got client" << endl;
        scheduleClient(conn, true);
    }
}

UNIXConnMgr::UNIXConnMgr(Controller *master, const string& address) {
    this->master = master;
    this->address = address;
}

UNIXConnMgr::~UNIXConnMgr() {
}

