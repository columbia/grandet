#include <boost/asio.hpp>
#include <arpa/inet.h>
#include "grandet.h"
#include "../../build/proto/grandet.pb.h"
#include <string.h>
#include <sstream>
#include <unistd.h>
#include <mutex>
#include <condition_variable>
#include "helper.h"

using namespace std;
using namespace grandet;
using boost::asio::ip::tcp;

// #define free(x)
// #define delete(x)

struct grandet_fetch_task_s {
    char *key;
    char *data_path;
    int   canceled;
    int   finished;
    long  size;
    int   rc;
    mutex lock;
};

struct grandet_sync_task_s {
    char *key;
    char *data;
    map<string, string> xattr;
    int   data_len;
};

static int _latency_requirement;
static int _bandwidth_requirement;
static string _base_path;
static mutex _conn_mutex;
static condition_variable _conn_avail_cv;
static vector<tcp::iostream *> _conns;
tcp::iostream *_fb_conn;
#define CONN_MAX 8
static int   _n_conn_alloc;

#define USE_CONN_POOL 1

void conn_get(tcp::iostream *&stream) {
#if USE_CONN_POOL
    {
        unique_lock<mutex> lock(_conn_mutex);
        while (_conns.size() == 0 && _n_conn_alloc >= CONN_MAX) {
            _conn_avail_cv.wait(lock);
        }

        if (_conns.size() > 0) {
            stream = _conns.back();
            _conns.pop_back();
        } else if (_n_conn_alloc < CONN_MAX) {
            ++ _n_conn_alloc;
            stream = new tcp::iostream();
            lock.unlock();
            stream->connect("127.0.0.1", "1481");
        }
    }
#else
    _conn_mutex.lock();
    if (_fb_conn == NULL) {
        _fb_conn = new tcp::iostream();
        _fb_conn->connect("127.0.0.1", "1481");
    }
    stream = _fb_conn;
#endif    
}

void conn_put(tcp::iostream *&stream) {
#if USE_CONN_POOL
    {
        unique_lock<mutex> lock(_conn_mutex);
        _conns.push_back(stream);
        if (_conns.size() == 1) {
            _conn_avail_cv.notify_one();
        }
    }
#else
    _conn_mutex.unlock();
#endif
}

template <class T>
inline bool _read_message(std::istream& stream, T& msg, bool debug = false) {
    uint32_t len;
    stream.read((char*)&len, 4);
    if (!stream) return false;
    len = ntohl(len);

    if (debug)
        DEBUG("read_message: get length %d\n", len);
        
    std::string buf;
    buf.resize(len);
    // XXX maybe incorrect
    stream.read(&buf[0], len);
    if (!stream) return false;

    return msg.ParseFromString(buf);
}

template <class T>
inline bool _write_message(std::ostream& stream, const T& msg, bool debug = false) {
    std::string buf;
    msg.SerializeToString(&buf);

    uint32_t len = buf.size();
    if (debug)
        DEBUG("write_message: get length %d\n", len);
    
    len = htonl(len);

    stream.write((char*)&len, 4);
    if (!stream) return false;

    stream.write(buf.c_str(), buf.size());
    if (!stream) return false;

    return true;
}

template<class T>
void _retry_write_message(tcp::iostream& stream, const T& msg) {
    while (true) {
        if (_write_message(stream, msg)) {
            return;
        }

        DEBUG("write failed, sleep and retry\n");
        stream.close();
        stream.clear();
        sleep(1);
        stream.connect("127.0.0.1", "1481");
    }
}

void
grandet_init(const char *base_path, int latency_requirement, int bandwidth_requirement) {
    _base_path = base_path;
    _latency_requirement = latency_requirement;
    _bandwidth_requirement = bandwidth_requirement;
    _n_conn_alloc = 0;
    _fb_conn = NULL;
}

int
grandet_fetch_task_start(const char *path, const char *key, const char *data_path, long size, grandet_fetch_task_t *ret) {
    grandet_fetch_task_t task = new grandet_fetch_task_s;
    task->rc = 1;

    task->data_path = strdup(data_path);
    task->key       = NULL;

    task->canceled = 0;
    if (access(task->data_path, F_OK) != -1) {
        task->finished = 1;
    } else {
        task->key = strdup(key);
        task->size = size;
        task->finished = 0;
    }

    if (ret) *ret = task;
    return task->finished;
}

void grandet_fetch_task_get(grandet_fetch_task_t task) {
    ++ task->rc;
}

void grandet_fetch_task_put(grandet_fetch_task_t task) {
    if (-- task->rc == 0) {
        INFO("drop fetch task %p\n", task);
        free(task->key);
        free(task->data_path);
        delete(task);
    }
}

void
grandet_fetch_task_cancel(grandet_fetch_task_t task) {
    task->lock.lock();
    task->canceled = 1;
    task->lock.unlock();
}

void
grandet_fetch_task_wait(grandet_fetch_task_t task) {
    task->lock.lock();
    if (task->finished == 0 && task->canceled == 0) {
        INFO("grandet_fetch_task processing\n");

        Request req;
        req.set_type(Request::GET);
        req.set_key(task->key);
        Request::Requirements *reqm = req.mutable_requirements();
        reqm->set_latency_required(_latency_requirement);
        reqm->set_bandwidth_required(_bandwidth_requirement);

        tcp::iostream *conn;
        conn_get(conn);
        if (!_write_message(*conn, req)) {
            DEBUG("grandet_fetch_task failed to write\n");
            task->canceled = 1;
            task->finished = 1;
            conn_put(conn);
            task->lock.unlock();
            return;
        }
        Response reply;
        if (!_read_message(*conn, reply)) {
            DEBUG("grandet_fetch_task failed to read response\n");
            task->canceled = 1;
            task->finished = 1;
            conn_put(conn);
            task->lock.unlock();
            return;
        }
        conn_put(conn);

        if (reply.status() != Response::OK) {
            DEBUG("grandet_fetch_task failed\n");
            task->canceled = 1;
        } else {
            ostringstream o;
            o << task->data_path << "_XXXXXX";
            char *tmp_path = strdup(o.str().c_str());
            int tmp_fh = mkstemp(tmp_path);

            const Value &val = reply.value();
            if (val.has_data()) {
                const string &data = val.data();
                INFO("grandet_fetch_task fetch data length %d\n", data.length());
                const char *buf = data.c_str();
                int length = data.length();
                if (length > task->size) length = task->size;
                while (length > 0) {
                    int w = write(tmp_fh, buf, length);
                    if (w < 0) {
                        DEBUG("grandet_fetch_task write failed\n");
                        task->canceled = 1;
                        break;
                    }
                    length -= w;
                    buf    += w;
                }
                ftruncate(tmp_fh, task->size);
            }

            close(tmp_fh);

            if (!task->canceled) {
                int r = rename(tmp_path, task->data_path);
                INFO("rename %s to %s ret %d\n", task->tmp_path, task->data_path, r == -1 ? errno : r);
            } else {
                unlink(tmp_path);
            }
        }

        task->finished = 1;
    }
    task->lock.unlock();
}

void
grandet_sync_task_start(const char *key, const char *data_path,
                        const map<string, string> &xattr, grandet_sync_task_t *ret) {
    grandet_sync_task_t task = new grandet_sync_task_s;
    task->xattr = xattr;

    int f = open(data_path, O_RDONLY);
    if (f == -1) {
        DEBUG("grandet_sync_task_start failed to open\n");
        if (ret) *ret = NULL;
        delete(task);
        return;
    }

    
    struct stat st;
    fstat(f, &st);
    char *buf = new char[st.st_size];

    char *cur = buf;
    int   length = st.st_size;
    while (length > 0) {
        int r = read(f, cur, length);
        if (r < 0) {
            DEBUG("grandet_sync_task_start failed to read\n");
            if (ret) *ret = NULL;
            delete(task);
            delete[] buf;
            return;
        }

        cur += r;
        length -= r;
    }

    task->key      = strdup(key);
    task->data     = buf;
    task->data_len = st.st_size;

    if (ret) *ret = task;
}

void
grandet_sync_task_wait(grandet_sync_task_t task) {
    if (task == NULL) return;
    
    Request req;
    req.set_type(Request::PUT);
    req.set_key(task->key);
    INFO("grandet_sync_task key = [%s]\n", task->key);
    req.mutable_value()->set_type(Value::DATA);

    req.mutable_value()->set_data(string(task->data, task->data_len));
    Request::Requirements *reqm = req.mutable_requirements();
    reqm->set_latency_required(_latency_requirement);
    reqm->set_bandwidth_required(_bandwidth_requirement);
    for (auto it = task->xattr.begin();
         it != task->xattr.end();
         ++ it) {
        auto *md = reqm->add_metadata();
        INFO("grandet_sync_task add metadata [%s]=[%s]\n", it->first.c_str(), it->second.c_str());
        md->set_meta_key(it->first);
        md->set_meta_value(it->second);
    }

    tcp::iostream *conn;
    conn_get(conn);
    if (!_write_message(*conn, req)) {
        DEBUG("grandet_sync_task failed - write\n");
        conn_put(conn);
        return;
    }
    
    Response reply;
    if (!_read_message(*conn, reply)) {
        DEBUG("grandet_sync_task failed - read\n");
        conn_put(conn);
        return;
    }
    conn_put(conn);
    
    int status = reply.status();
    if (status != Response::OK) {
        DEBUG("grandet_sync_task failed\n");
    }
}

void
grandet_sync_task_end(grandet_sync_task_t task) {
    if (task == NULL) return;
    
    free(task->key);
    delete[] task->data;
    delete(task);
}

struct grandet_remove_task_s {
    char *key;
    char *data_path;
};

void
grandet_remove_task_start(const char *key, const char *data_path, grandet_remove_task_t *ret) {
    grandet_remove_task_t task = new grandet_remove_task_s;
    task->key = strdup(key);
    task->data_path = strdup(data_path);
    if (ret) *ret = task;
}

void
grandet_remove_task_wait(grandet_remove_task_t task) {
    unlink(task->data_path);
    
    Request req;
    req.set_type(Request::DEL);
    req.set_key(task->key);
    
    tcp::iostream *conn;
    conn_get(conn);
    if (!_write_message(*conn, req)) {
        DEBUG("grandet_remove_task failed\n");
        conn_put(conn);
    } else {

        Response reply;
        if (!_read_message(*conn, reply)) {
            DEBUG("grandet_remove_task failed\n");
        } else if (reply.status() != Response::OK) {
            DEBUG("grandet_remove_task failed\n");
        } else {
            INFO("grandet_remove_task successed\n");
        }

        conn_put(conn);
    }
}

void
grandet_remove_task_end(grandet_remove_task_t task) {
    free(task->key);
    free(task->data_path);
    delete(task);
}
