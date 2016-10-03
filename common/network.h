#ifndef _GRANDET_NETWORK_H_
#define _GRANDET_NETWORK_H_

#include <arpa/inet.h>
#if USE_ASIO
#include <istream>
#else
#include <unistd.h>
#endif

#ifdef REALDEBUG
#include "log.h"
#endif

#if USE_ASIO
typedef std::istream& flow_t;
#else
typedef int flow_t;
#endif

#define USE_RECV

template <class T>
inline bool ReadMessage(flow_t stream, T& msg) {
    uint32_t len;
#if USE_ASIO
    stream.read((char*)&len, 4);
    if (!stream) return false;
#else
    if (read(stream, (char*)&len, 4) != 4) return false;
#endif
    len = ntohl(len);
#ifdef REALDEBUG
    Dbg() << "got len: " << len << std::endl;
#endif

    char *buf = (char*)malloc(len);
#if USE_ASIO
    // XXX maybe incorrect
    stream.read(buf, len);
    if (!stream) {
        free(buf);
        return false;
    }
#else
#ifdef USE_RECV
    if (recv(stream, buf, len, MSG_WAITALL) != len) {
        free(buf);
        return false;
    }
#else
    int left = len, cur = 0;
    char *ptr = buf;
    while (left > 0) {
        cur = read(stream, ptr, left);
        if (cur < 0) {
            free(buf);
            return false;
        }
        ptr += cur;
        left -= cur;
    }
#endif
#endif
#ifdef REALDEBUG
    Dbg() << "got buf" << std::endl;
#endif

    bool ret = msg.ParseFromArray(buf, len);
    free(buf);
    return ret;
}

template <class T>
inline bool WriteMessage(flow_t stream, const T& msg) {
    std::string buf;
    msg.SerializeToString(&buf);

    uint32_t len = buf.size();
    len = htonl(len);

#if USE_ASIO
    stream.write((char*)&len, 4);
    if (!stream) return false;
#else
//    if (write(stream, (char*)&len, 4) != 4) return false;
    if (send(stream, (char*)&len, 4, MSG_NOSIGNAL) != 4) return false;
#endif
#ifdef REALDEBUG
    Dbg() << "sent len: " << ntohl(len) << std::endl;
#endif

#if USE_ASIO
    stream.write(buf.c_str(), buf.size());
    if (!stream) return false;
#else
    if (write(stream, buf.c_str(), buf.size()) != buf.size()) return false;
#endif
#ifdef REALDEBUG
    Dbg() << "sent data: " << ntohl(len) << std::endl;
#endif

    return true;
}

#endif // _GRANDET_NETWORK_H_
