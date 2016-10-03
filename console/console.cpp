#include "log.h"
#include "grandet.pb.h"
#include "network.h"
#include <iostream>

#if USE_ASIO
#include <boost/asio.hpp>
using boost::asio::ip::tcp;
using boost::asio::local::stream_protocol;
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#endif

#include <readline/readline.h>
#include <readline/history.h>
#include "range.h"
using namespace std;
using namespace grandet;

const char *server_addr = "/tmp/grandet_socket";

int CreateConn(const char *addr) {
    int stream = socket(PF_UNIX, SOCK_STREAM, 0);
    if (stream < 0) return stream;

    struct sockaddr_un server;
    server.sun_family = AF_UNIX;
    strcpy(server.sun_path, addr);
    int ret = connect(stream, (struct sockaddr *)&server, sizeof(struct sockaddr_un));
    if (ret < 0) return ret;
    return stream;
}

vector<string> GetArgs(const string& args) {
    int pos = 0;
    int prevPos = 0;
    vector<string> result;
    while ((pos = args.find(' ', prevPos)) != string::npos) {
        string part = args.substr(prevPos, pos - prevPos);
        result.push_back(part);
        prevPos = pos + 1;
    }
    result.push_back(args.substr(prevPos));
    return result;
}

template<class T>
void RetryWriteMessage(flow_t stream, const T& msg) {
    while (true) {
        if (WriteMessage(stream, msg)) {
            return;
        }

        Warn() << "fail to write. close and retry." << endl;
#if USE_ASIO
        stream.close();
        stream.clear();
        sleep(1);
        stream.connect(stream_protocol::endpoint("/tmp/grandet_socket"));
#else
        close(stream);
        sleep(1);
        stream = CreateConn(server_addr);
#endif
    }
}

void help() {
    Err() << "available commands:\n"
        "g <key>              get object\n"
        "p <key> <value>      put object\n"
        "s <key>              get object size\n"
        "k                    list objects\n"
        "d <key>              delete object\n"
        "l <key>              lock object\n"
        "u <key>              unlock object\n"
        "r                    reset, clear everything\n"
        "i                    get distribution information\n"
        "a                    get accounting information\n"
        "q                    quit\n";
}

bool runCmd(string line, flow_t stream) {
    switch(line[0]) {
        case 'g':
            {
                if (line.size() < 3) {
                    Err() << "missing args" << endl;
                    break;
                }
                line = line.substr(2);
                vector<string> args = GetArgs(line);
                string key = args[0];
                Range range;
                if (args.size() >= 2) {
                    range.start = stoi(args[1]);
                }
                if (args.size() >= 3) {
                    range.end = stoi(args[2]);
                }

                Request req;
                req.set_type(Request::GET);
                req.set_key(key);
                Request::Requirements* reqm = req.mutable_requirements();;
                reqm->set_latency_required(100);
                reqm->set_bandwidth_required(1);
                if (range.start) {
                    reqm->mutable_range()->set_start(*range.start);
                }
                if (range.end) {
                    reqm->mutable_range()->set_end(*range.end);
                }

                RetryWriteMessage(stream, req);

                Response reply;
                ReadMessage(stream, reply);

                if (reply.status() != Response::OK) {
                    Err() << "Error!" << endl;
                } else {
                    const Value& val = reply.value();
                    cout << val.data() << endl;
                }
                break;
            }
        case 'p':
            {
                if (line.size() < 3) {
                    Err() << "missing args" << endl;
                    break;
                }
                line = line.substr(2);
                vector<string> args = GetArgs(line);
                if (args.size() < 2) {
                    Err() << "format error, missing ' '" << endl;
                    break;
                }
                string key = args[0];
                string value = args[1];
                int latencyReq = 100;
                if (args.size() > 2) {
                    latencyReq = stoi(args[2]);
                }

                Request req;
                req.set_type(Request::PUT);
                req.set_key(key);
                req.mutable_value()->set_type(Value::DATA);
                req.mutable_value()->set_data(value);
                Request::Requirements* reqm = req.mutable_requirements();;
                reqm->set_latency_required(latencyReq);
                reqm->set_bandwidth_required(1);

                RetryWriteMessage(stream, req);

                Response reply;
                ReadMessage(stream, reply);

                if (reply.status() != Response::OK) {
                    Err() << "Error!" << endl;
                } else {
                    cout << "OK!" << endl;
                }
                break;
            }
        case 's':
            {
                if (line.size() < 3) {
                    Err() << "missing args" << endl;
                    break;
                }
                line = line.substr(2);
                vector<string> args = GetArgs(line);
                if (args.size() < 1) {
                    Err() << "format error, missing ' '" << endl;
                    break;
                }
                string key = args[0];

                Request req;
                req.set_type(Request::SIZE);
                req.set_key(key);
                RetryWriteMessage(stream, req);

                Response reply;
                ReadMessage(stream, reply);
                if (reply.status() != Response::OK) {
                    Err() << "Error!" << endl;
                } else {
                    cout << reply.value().size() << endl;
                }
                break;
            }
        case 'k':
            {
                Request req;
                req.set_type(Request::KEYS);
                RetryWriteMessage(stream, req);

                Response reply;
                ReadMessage(stream, reply);
                cout << reply.value().data();
                break;
            }
        case 'd':
            {
                if (line.size() < 3) {
                    Err() << "missing args" << endl;
                    break;
                }
                line = line.substr(2);
                vector<string> args = GetArgs(line);
                if (args.size() < 1) {
                    Err() << "format error, missing ' '" << endl;
                    break;
                }
                string key = args[0];

                Request req;
                req.set_type(Request::DEL);
                req.set_key(key);
                RetryWriteMessage(stream, req);

                Response reply;
                ReadMessage(stream, reply);
                if (reply.status() != Response::OK) {
                    Err() << "Error!" << endl;
                } else {
                    cout << "OK!" << endl;
                }
                break;
            }
        case 'l':
            {
                if (line.size() < 3) {
                    Err() << "missing args" << endl;
                    break;
                }
                line = line.substr(2);
                vector<string> args = GetArgs(line);
                if (args.size() < 1) {
                    Err() << "format error, missing ' '" << endl;
                    break;
                }
                string key = args[0];

                Request req;
                req.set_type(Request::LOCK);
                req.set_key(key);
                RetryWriteMessage(stream, req);

                Response reply;
                ReadMessage(stream, reply);
                if (reply.status() != Response::OK) {
                    Err() << "Error!" << endl;
                } else {
                    cout << "OK!" << endl;
                }
                break;
            }
        case 'u':
            {
                if (line.size() < 3) {
                    Err() << "missing args" << endl;
                    break;
                }
                line = line.substr(2);
                vector<string> args = GetArgs(line);
                if (args.size() < 1) {
                    Err() << "format error, missing ' '" << endl;
                    break;
                }
                string key = args[0];

                Request req;
                req.set_type(Request::UNLOCK);
                req.set_key(key);
                RetryWriteMessage(stream, req);

                Response reply;
                ReadMessage(stream, reply);
                if (reply.status() != Response::OK) {
                    Err() << "Error!" << endl;
                } else {
                    cout << "OK!" << endl;
                }
                break;
            }
        case 'r':
            {
                Request req;
                req.set_type(Request::RESET);
                RetryWriteMessage(stream, req);

                Response reply;
                ReadMessage(stream, reply);
                if (reply.status() != Response::OK) {
                    Err() << "Error!" << endl;
                } else {
                    cout << "OK!" << endl;
                }
                break;
            }
        case 'i':
            {
                Request req;
                req.set_type(Request::DIST);
                RetryWriteMessage(stream, req);

                Response reply;
                ReadMessage(stream, reply);
                cout << reply.value().data();
                break;
            }
        case 'a':
            {
                Request req;
                req.set_type(Request::ACCOUNT);
                RetryWriteMessage(stream, req);

                Response reply;
                ReadMessage(stream, reply);
                if (reply.status() != Response::OK) {
                    Err() << "Error!" << endl;
                } else {
                    cout << "OK!" << endl;
                }
                break;
            }
        case 'x':
            {
                Request req;
                req.set_type(Request::EXIT);
                RetryWriteMessage(stream, req);
                break;
            }
        case 'q':
            return true;
        case'h':
            help();
            break;
        default:
            Err() << "unknown command " << line[0] << endl;
    }
    return false;
}

int main(int argc, char **argv) {
#if USE_ASIO
    boost::asio::io_service io_service;
/*    tcp::iostream stream;
    stream.connect("127.0.0.1", "1481");
    stream.rdbuf()->set_option(boost::asio::ip::tcp::no_delay(true));*/
    stream_protocol::iostream stream;
    stream.connect(stream_protocol::endpoint("/tmp/grandet_socket"));
#else
    int stream = CreateConn("/tmp/grandet_socket");
    if (stream < 0) {
        Err() << "fail to connect" << endl;
        return 1;
    }
#endif

    if (argc > 1) {
        for (int i=1; i<argc; i++) runCmd(string(argv[i]), stream);
        return 0;
    }

    while (cin.good()) {
        char* xline = readline("grandet>");
        if (!xline) break;
        add_history(xline);
        string line(xline);
        free(xline);
        if (line == "") {
            continue;
        }

        if (runCmd(line, stream)) break;
    }
}
