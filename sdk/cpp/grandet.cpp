#include "grandet.h"

#include "grandet.pb.h"
#include "network.h"

#if USE_ASIO
#include <boost/asio.hpp>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif

#include <string>

#if USE_ASIO
using boost::asio::ip::tcp;
using boost::asio::local::stream_protocol;
#endif

using namespace grandet;
using namespace std;

class GrandetClientImpl : public GrandetClient {
public:
    GrandetClientImpl() {}

    void connect(const string& server, const string& port) {
#if USE_ASIO
        tcp::iostream* tcp_stream = new tcp::iostream();
        tcp_stream->connect(server, port);
        tcp_stream->rdbuf()->set_option(boost::asio::ip::tcp::no_delay(true));
        stream = tcp_stream;
#else
        sock = socket(PF_INET, SOCK_STREAM, 0);
        struct sockaddr_in sin;
        memset(&sin, 0, sizeof(sin));
        sin.sin_family = AF_INET;
        sin.sin_port = htons(atoi(port.c_str()));

        struct hostent *hostinfo = gethostbyname(server.c_str());
        if (hostinfo == NULL) {
            return;
        }
        sin.sin_addr = *(struct in_addr *)hostinfo->h_addr;
        ::connect(sock, (struct sockaddr *)&sin, sizeof(struct sockaddr_in));
#endif
    }

    void connect_unix(const string& server) {
#if USE_ASIO
        stream_protocol::iostream *unix_stream = new stream_protocol::iostream();
        stream_protocol::endpoint ep(server);
        unix_stream->connect(ep);
        stream = unix_stream;

#else
        sock = socket(PF_UNIX, SOCK_STREAM, 0);
        struct sockaddr_un sa;
        sa.sun_family = AF_UNIX;
        strcpy(sa.sun_path, server.c_str());
        ::connect(sock, (struct sockaddr *)&sa, sizeof(struct sockaddr_un));
#endif
    }

    ErrorCode put(const string& key, const string& value,
            int latency_required, int bandwidth_required) {
        req.set_type(Request::PUT);
        req.set_key(key);
        req.mutable_value()->set_type(Value::DATA);
        req.mutable_value()->set_data(value);
        Request::Requirements* reqm = req.mutable_requirements();;
        reqm->set_latency_required(latency_required);
        reqm->set_bandwidth_required(bandwidth_required);

#if USE_ASIO
#error Not implemented
#else
        WriteMessage(sock, req);

        ReadMessage(sock, reply);
#endif

        if (reply.status() != Response::OK) {
            return ERROR_FAIL;
        } else {
            return ERROR_OK;
        }
    }

    StatusOr<string> get(const string& key, int latency_required,
            int bandwidth_required, int range_start, int range_end) {
        req.set_type(Request::GET);
        req.set_key(key);
        Request::Requirements* reqm = req.mutable_requirements();;
        reqm->set_latency_required(latency_required);
        reqm->set_bandwidth_required(bandwidth_required);
        if (range_start) {
            reqm->mutable_range()->set_start(range_start);
        }
        if (range_end) {
            reqm->mutable_range()->set_end(range_end);
        }

#if USE_ASIO
#error Not implemented
#else
        WriteMessage(sock, req);

        ReadMessage(sock, reply);
#endif

        if (reply.status() != Response::OK) {
            return ERROR_FAIL;
        } else {
            Value* val = reply.mutable_value();
            return *val->mutable_data();
        }
        return ERROR_OK;
    }

private:
#if USE_ASIO
    boost::asio::io_service io_service;
    std::iostream *stream;
#else
    int sock;
#endif
    Request req;
    Response reply;

    string server;
    string port;
};

GrandetClient* CreateGrandetClient() {
    return new GrandetClientImpl();
}
