#define FUSE_USE_VERSION 30
#include <fuse.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <attr/xattr.h>

#include <boost/lexical_cast.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <sstream>
#include <vector>
#include <algorithm>
#include <mutex>
#include <hiredis.h>
#include <set>
#include <condition_variable>

#include "inl.h"
#include "helper.h"

using namespace std;

// #define free(x)
// #define delete(x)

#define FLUSH_CACHE_ON_RELEASE

string        _data_store_base;
list_entry_s  _idle_list;
long          _evicting_threshold_bytes;
long          _cache_total_size_bytes;
mutex         _files_lock;
file_entry_t  _root;
map<string, file_entry_t> _files;

#define FILE_DESC_UPPER_LIMIT (1 << 20)
file_entry_t fd_fe[FILE_DESC_UPPER_LIMIT];
long         fd_touch_size[FILE_DESC_UPPER_LIMIT];
redisContext *_redis_conn;

#ifdef DUMP_BUF
void dump_buf(const char *buf, size_t size) {
    DEBUG("BUF %lu: ", size);
    for (int i = 0; i < size; i++) {
        fprintf(stderr, "%02x ", (unsigned char)buf[i]);
    }
    fprintf(stderr, "END BUF\n");
}
#endif

file_entry_t
new_fe(void) {
    file_entry_t fe = new file_entry_s();

    fe->rc = 1;
    fe->parent = NULL;
    
    memset(&fe->st, 0, sizeof(fe->st));
    fe->st.st_uid = getuid();
    fe->st.st_gid = getgid();
    clock_gettime(CLOCK_REALTIME, &fe->st.st_atim);
    memcpy(&fe->st.st_mtim, &fe->st.st_atim, sizeof(struct timespec));
    memcpy(&fe->st.st_ctim, &fe->st.st_atim, sizeof(struct timespec));
    
    list_init(&fe->c_node);
    list_init(&fe->c_list);
    list_init(&fe->u_node);
    
    fe->fetch_task = NULL;

    fe->deleted       = 0;
    fe->fetched       = 0;
    fe->content_dirty = 0;
    fe->open_rc       = 0;
    return fe;
}

// assuming the lock is fetched
void fe_get(file_entry_t fe) {
    ++ fe->rc;
}

// assuming the lock is fetched
void fe_put(file_entry_t fe) {
    if (-- fe->rc == 0) {
        if (!fe->deleted || fe->open_rc || fe->content_dirty) {
            DEBUG("unwanted free %s %d %d %d\n",
                  fe->link.c_str(),
                  fe->deleted, fe->open_rc, fe->content_dirty);
        }
        
        if (!list_empty(&fe->u_node))
            list_del_init(&fe->u_node);
        delete(fe);
    }
}

void
split_path(const string &path, string &parent, string &base) {
    int len = path.length();
    if (path[len - 1] == '/') -- len;
    for (; len > 0; -- len)
        if (path[len - 1] == '/') {
            -- len;
            break;
        }
    parent.assign(path, 0, len ? len : 1);
    base.assign(path, len + 1, path.length() - len - 1);
}

void
get_data_path(string &ret, const string &base, const char *link) {
    ostringstream o;
    o << base << '/' << link;
    ret = o.str();
}

// assuming _files_lock is acquired
void
cache_add(long size) {
    _cache_total_size_bytes += size;
    while (_cache_total_size_bytes > _evicting_threshold_bytes) {
        list_entry_s *le = list_next(&_idle_list);
        if (le == &_idle_list) break;
        file_entry_t fe = CONTAINER_OF(le, file_entry_s, u_node);
        // all file in the list should be fetched, clean files and no one opens it
        if (!fe->fetched || fe->content_dirty || fe->open_rc > 0) {
            DEBUG("ERR: unwanted item %s in idle list, "
                  "fetched = %d, content_dirty = %d, open_rc = %d\n",
                  fe->link.c_str(), fe->fetched, fe->content_dirty, fe->open_rc);
#if VDEBUG
            abort();
#endif
        } else {
            INFO("evicting %s %ld\n", fe->link.c_str(), fe->st.st_size);
            // safely throw the data but keep nodes
            fe->fetched = 0;

            if (!list_empty(&fe->u_node)) {
                INFO("  remove %s from idle_list (evict)\n", fe->link.c_str());
                list_del_init(&fe->u_node);
            }

            string data_path;
            get_data_path(data_path, _data_store_base, fe->link.c_str());

            unlink(data_path.c_str());
            _cache_total_size_bytes -= fe->st.st_size;
        }
        list_del_init(le);
    }
    INFO("after add: cache_total = %ld\n", _cache_total_size_bytes);
}

void
cache_remove(long size) {
    _cache_total_size_bytes -= size;
    INFO("after remove: cache_total = %ld\n", _cache_total_size_bytes);
}

// assume _files_lock hold
int get_real_path(file_entry_t fe, string &path) {
    ostringstream o;
    vector<string> path_vec;
    file_entry_t cur = fe;
    
    while (cur != _root) {
        if (cur == NULL) {
            DEBUG("get_real_path: trace back to NULL node");
            return -1;
        }

        path_vec.push_back(cur->name);
        cur = cur->parent;
    }

    if (fe == _root) {
        path = "/";
    } else {
        for (int i = path_vec.size() - 1; i >= 0; -- i) {
            o << "/" << path_vec[i];
        }
        path = o.str();
    }

    return 0;
};

void check_fe(file_entry_t fe) {
    if (fe == NULL) {
        DEBUG("check_fe: get a NULL node\n");
        return;
    }

    string path;
    if (get_real_path(fe, path)) {
        DEBUG("cannot get real path\n");
        return;
    }

    int in_tree;
    auto it = _files.find(path);
    in_tree = (it != _files.end() &&
               it->second == fe);
    
    DEBUG("check_fe: file [%s] size = %ld, rc = %d, fetched = %d, "
          "deleted = %d, open_rc = %d, in_tree = %d\n",
          path.c_str(),
          fe->st.st_size,
          fe->rc, fe->fetched, fe->deleted, fe->rc, in_tree);
}

class UpdateQueue {
private:
    mutex cur_lock;
    set<file_entry_t> queue[3];
    int               cur_index;
    set<file_entry_t> *cur_set;
    condition_variable cond;
public:
    UpdateQueue() {
        cur_set = &queue[0];
        cur_index = 0;
    }

    // assume the _files_lock is held
    void add(file_entry_t fe) {
        fe_get(fe);
        cur_lock.lock();
        cur_set->insert(fe);
        cur_lock.unlock();
        cond.notify_all();
    }

    // running in a dedicated thread
    void process() {
        int clear_index;
        {
            unique_lock<mutex> auto_lock(cur_lock);
            cond.wait(auto_lock);
            clear_index = (cur_index + 2) % 3;
            int next_index = (cur_index + 1) % 3;
            cur_index = next_index;
            cur_set = &queue[next_index];
        }
        auto back_set = &queue[clear_index];

        auto it = back_set->begin();
        while (it != back_set->end()) {
            file_entry_t fe = *it;
            grandet_sync_task_t sync_task = NULL;
            grandet_remove_task_t remove_task = NULL;
            _files_lock.lock();
            INFO("processing %s\n", fe->link.c_str());
            if (fe->deleted) {
                INFO("processing deleted fe, size=%ld, fetched=%d, open_rc=%d\n", fe->st.st_size, fe->fetched, fe->open_rc);
            }
            if (fe->deleted && fe->open_rc == 0) {
                // when it is out of tree, no one can open it anymore 
                if (fe->deleted == 1) {
                    string data_path;
                    if (fe->fetched)
                        cache_remove(fe->st.st_size);
                    get_data_path(data_path, _data_store_base, fe->link.c_str());
                    grandet_remove_task_start(fe->link.c_str(), data_path.c_str(), &remove_task);
                    grandet_remove_task_wait(remove_task);
                    grandet_remove_task_end(remove_task);
                    
                    fe->deleted = 2; // avoiding repeated deletion
                }
                fe_put(fe);
            } else {
                // Update
                if (fe->content_dirty) {
                    if (!S_ISREG(fe->st.st_mode)) {
                        DEBUG("flush a non-regular file");
                    }
                    
                    string data_path;
                    get_data_path(data_path, _data_store_base, fe->link.c_str());
                    
                    fe->content_dirty = 0;
                    grandet_sync_task_start(fe->link.c_str(), data_path.c_str(), fe->xattr, &sync_task);
                    
                    _files_lock.unlock();
                    grandet_sync_task_wait(sync_task);
                    grandet_sync_task_end(sync_task);
                    _files_lock.lock();

                    // during the unlock and lock, there may be some
                    // thread writing data in and close, causing
                    // content_dirty be 1 again. Or some one can even
                    // delete it.  In those cases, it will appear in
                    // update queue again.
                    if (fe->open_rc == 0 && fe->fetched == 1 &&
                        fe->content_dirty == 0 && fe->deleted == 0) {
                        // DEBUG("add %s to idle list\n", fe->link.c_str());
                        // LRU
                        if (!list_empty(&fe->u_node))
                            list_del_init(&fe->u_node);
                        INFO("add %s to idle_list (dirty)\n", fe->link.c_str());
                        list_add_before(&_idle_list, &fe->u_node);
                    }
                } else if (fe->fetched == 1 && fe->open_rc == 0) {
                    if (!list_empty(&fe->u_node))
                        list_del_init(&fe->u_node);
                    INFO("add %s to idle_list (clean)\n", fe->link.c_str());
                    list_add_before(&_idle_list, &fe->u_node);
                }
                fe_put(fe);
            }
            _files_lock.unlock();
            ++ it;
        }
        back_set->clear();
    }
} _update_queue;

double get_epoch() {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);

    return ts.tv_sec + ts.tv_nsec * 0.000000001;
}

pthread_t update_thread;

void *
update_thread_func(void *_) {
    while (true) {
        _update_queue.process();
    }
}

// unlink fe from tree with lock held
static void unlink_fe_unsafe(const char *path, file_entry_t fe) {
    if (S_ISDIR(fe->st.st_mode) || S_ISLNK(fe->st.st_mode)) {

        list_del(&fe->c_node);
        fe->parent = NULL;
        _files.erase(path);

        fe_put(fe);

    } else if (S_ISREG(fe->st.st_mode)) {
        
        if (fe->fetch_task != NULL) {
            grandet_fetch_task_cancel(fe->fetch_task);
        }
        
        list_del(&fe->c_node);
        fe->parent = NULL;
        _files.erase(path);
        fe->deleted = 1;
        
        _update_queue.add(fe);
        fe_put(fe);
        
    } else {
        DEBUG("try to unlink file that is not dir, reg nor lnk\n");
    }
}

static void redis_move(const char *from, const char *to) {
    redisReply *reply;
    
    reply = (redisReply *)redisCommand(_redis_conn, "RENAME f:%s f:%s", from, to);
    freeReplyObject(reply);

    reply = (redisReply *)redisCommand(_redis_conn, "RENAME x:%s x:%s", from, to);
    freeReplyObject(reply);
}

static void redis_remove(const char *path) {
    redisReply *reply;
    
    reply = (redisReply *)redisCommand(_redis_conn, "DEL f:%s", path);
    freeReplyObject(reply);
    
    reply = (redisReply *)redisCommand(_redis_conn, "DEL x:%s", path);
    freeReplyObject(reply);
}

void
redis_parse(const char *path, file_entry_t fe) {
    redisReply *reply = (redisReply *)redisCommand(_redis_conn, "GET f:%s", path);
    
    memcpy(&fe->st, reply->str, sizeof(struct stat));
    // byte-order?
    fe->link.assign(reply->str + sizeof(struct stat), reply->len - sizeof(struct stat));
    freeReplyObject(reply);

    reply = (redisReply *)redisCommand(_redis_conn, "HGETALL x:%s", path);
    int i = 0;
    while (i + 1 < reply->elements) {
        // assuming null-terminating in name
        fe->xattr[reply->element[i]->str] = string(reply->element[i + 1]->str, reply->element[i + 1]->len);
        i += 2;
    }
    freeReplyObject(reply);
}

void
redis_sync(const char *path, file_entry_t fe) {
    int buf_size = sizeof(struct stat) + fe->link.length();
    char *buf = new char[buf_size];

    struct stat st = fe->st;
    // byte-order?
    memcpy(buf, &st, sizeof(struct stat));
    memcpy(buf + sizeof(struct stat), fe->link.c_str(), fe->link.length());
    redisReply *reply = (redisReply *)redisCommand(_redis_conn, "SET f:%s %b", path, buf, (size_t)buf_size);
    freeReplyObject(reply);

    // xattr
    auto it = fe->xattr.begin();
    while (it != fe->xattr.end()) {
        redisReply *reply = (redisReply *)
            redisCommand(_redis_conn,
                         "HSET x:%s %s %b",
                         path, it->first.c_str(),
                         it->second.c_str(), it->second.length());
        freeReplyObject(reply);

        ++ it;
    }

    delete [] buf;
}

static void _sync_fe_unsafe(const char *path, file_entry_t fe, fuse_file_info *fi) {
    struct stat st;
    if (fd_touch_size[fi->fh] > fe->st.st_size && fe->fetched) {
        cache_remove(fe->st.st_size);
        cache_add(st.st_size);
        fe->st.st_size = fd_touch_size[fi->fh];
    }

    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    memcpy(&fe->st.st_atim, &ts, sizeof(ts));
    if (fe->content_dirty) {
        memcpy(&fe->st.st_mtim, &ts, sizeof(ts));
    }
            
    redis_sync(path, fe);
    INFO(" sync_fe: added %s to queue\n", path);
    _update_queue.add(fe);
}

// interface code begins here

static int _getattr(const char *path, struct stat *stbuf) {
    INFO("getattr [%s]\n", path);
    int ret = 0;
    
    _files_lock.lock();
    
    auto it = _files.find(path);
    if (it == _files.end()) {
        ret = -ENOENT;
    } else {
        file_entry_t fe = it->second;
        memcpy(stbuf, &fe->st, sizeof(struct stat));
        INFO("  size = %ld\n", stbuf->st_size);
    }
    
    _files_lock.unlock();
    return ret;
}

static int _readdir(const char *path, void *buf, fuse_fill_dir_t filler,
             off_t offset, struct fuse_file_info *fi) {
    INFO("readdir [%s]\n", path);
    (void) offset;
    (void) fi;
    int ret = 0;
    
    _files_lock.lock();

    auto it = _files.find(path);
    if (it == _files.end()) {
        ret = -ENOENT;
    } else {
        file_entry_t fe = it->second;

        filler(buf, ".", NULL, 0);
        filler(buf, "..", NULL, 0);

        for (list_entry_s *le = list_next(&fe->c_list);
             le != &fe->c_list; le = list_next(le)) {
            file_entry_t c = CONTAINER_OF(le, file_entry_s, c_node);
            filler(buf, c->name.c_str(), NULL, 0);
        }
    }
    
    _files_lock.unlock();
    return ret;
}

static int _access(const char *path, int mask) {
    INFO("access [%s]\n", path);
    
    int ret = 0;
    _files_lock.lock();
    if (_files.find(path) == _files.end()) {
        ret = -ENOENT;
    }
    _files_lock.unlock();

    if (ret < 0) {
        DEBUG("access [%s] error %d\n", path, -ret);
    }
    return ret;
}

static int _readlink(const char *path, char *buf, size_t size) {
    int ret = 0;
    
    _files_lock.lock();
    auto it = _files.find(path);
    if (it == _files.end()) {
        ret = -ENOENT;
    } else {
        file_entry_t fe = it->second;
        if (!S_ISLNK(fe->st.st_mode)) {
            ret = -EINVAL;
        } else {
            snprintf(buf, size, "%s", fe->link.c_str());
        }
    }
    _files_lock.unlock();

    return ret;
}

static int _mknod(const char *path, mode_t mode, dev_t rdev) {
    return -EPERM;
}

static int _mkdir(const char *path, mode_t mode) {
    INFO("mkdir [%s]\n", path);
    int ret = 0;
    string parent_path;
    string basename;
    split_path(path, parent_path, basename);
    INFO("path [%s], parent [%s], base [%s]\n", path, parent_path.c_str(), basename.c_str());
    
    _files_lock.lock();
    map<string, file_entry_t>::iterator p_it, t_it;
    if ((p_it = _files.find(parent_path)) == _files.end()) {
        ret = -ENOENT;
    } else if ((t_it = _files.find(path)) != _files.end()) {
        // XXX
        ret = -EINVAL;
    } else {
        file_entry_t p = p_it->second;
        file_entry_t fe = new_fe();
        fe->name = basename;
        fe->st.st_mode = S_IFDIR | (mode & 0777);

        fe->parent = p;
        
        list_add(&p->c_list, &fe->c_node);
        _files[path] = fe;

        redis_sync(path, fe);
    }
    _files_lock.unlock();
    return ret;
}

static int _unlink(const char *path) {
    INFO("unlink [%s]\n", path);
    
    int ret = 0;
    
    _files_lock.lock();
    auto it = _files.find(path);
    if (it == _files.end()) {
        ret = -ENOENT;
    } else {
        file_entry_t fe = it->second;

        if (S_ISDIR(fe->st.st_mode)) {
            // XXX ERR CODE
            ret = -EINVAL;
        } else {

            unlink_fe_unsafe(path, fe);
            redis_remove(path);
        }
    }
    _files_lock.unlock();

    return ret;
}

static int _rmdir(const char *path) {
    INFO("rmdir [%s]\n", path);
    
    int ret = 0;

    _files_lock.lock();
    auto it = _files.find(path);
    if (it == _files.end()) {
        ret = -ENOENT;
    } else {
        file_entry_t fe = it->second;

        if (!S_ISDIR(fe->st.st_mode)) {
            // XXX
            ret = -EINVAL;
        } else if (!list_empty(&fe->c_list)) {
            // non-empty directory
            ret = -EINVAL;
        } else {
            unlink_fe_unsafe(path, fe);
            redis_remove(path);
        }
    }
    _files_lock.unlock();

    return ret;
}

static int _symlink(const char *from, const char *to) {
    INFO("symlink [%s] => [%s]\n", from, to);
    
    int ret = 0;
    string t_p, t_b;
    split_path(to, t_p, t_b);

    _files_lock.lock();
    map<string, file_entry_t>::iterator p_it, t_it;
    if ((p_it = _files.find(t_p)) == _files.end()) {
        ret = -ENOENT;
    } else if ((t_it = _files.find(to)) != _files.end()) {
        ret = -EINVAL;
    } else {
        file_entry_t fe = p_it->second;
        if (!S_ISDIR(fe->st.st_mode)) {
            // XXX
            ret = -EINVAL;
        } else {
            file_entry_t ce = new_fe();
            ce->name = t_b;
            ce->link = from;
            ce->st.st_mode = S_IFLNK | 0777;
            ce->st.st_size = strlen(from);

            ce->parent = fe;
            list_add(&fe->c_list, &ce->c_node);
            _files[to] = ce;

            redis_sync(to, ce);
        }
    }
    _files_lock.unlock();
    return ret;
}

static int _rename(const char *from, const char *to) {
    INFO("rename [%s] to [%s]\n", from, to);
    
    int ret = 0;
    string t_p, t_b;
    split_path(to, t_p, t_b);

    _files_lock.lock();
    map<string, file_entry_t>::iterator s_it, p_it, t_it;
    s_it = _files.find(from);
    p_it = _files.find(t_p);
    t_it = _files.find(to);
    
    if (s_it == _files.end()) {
        ret = -ENOENT;
    } else if (p_it == _files.end()) {
        ret = -ENOENT;
    } else {
        file_entry_t p = p_it->second;

        if (!S_ISDIR(p->st.st_mode)) {
            ret = -EINVAL;
        } else {

            file_entry_t fe = s_it->second;
            file_entry_t old_fe = t_it == _files.end() ? NULL : t_it->second;

            list_del(&fe->c_node);
            _files.erase(from);

            if (old_fe) {
                unlink_fe_unsafe(to, old_fe);
            }
            
            fe->name = t_b;
            fe->parent = p;
            list_add(&p->c_list, &fe->c_node);
            _files[to] = fe;

            redis_move(from, to);
        }
    }
    _files_lock.unlock();
    if (ret < 0) {
        DEBUG("rename [%s] to [%s] error %d\n", from, to, -ret);
    }
    return ret;
}

static int _chmod(const char *path, mode_t mode) {
    INFO("chmod [%s] to %04o\n", path, mode);

    int ret = 0;

    _files_lock.lock();
    auto it = _files.find(path);
    if (it == _files.end()) {
        ret = -ENOENT;
    } else {
        file_entry_t fe = it->second;
        fe->st.st_mode = (fe->st.st_mode & ~0777) | (mode & 0x777);

        redis_sync(path, fe);
    }
    _files_lock.unlock();

    return ret;
}

static int _chown(const char *path, uid_t uid, gid_t gid) {
    INFO("chown [%s] to uid = %d, gid = %d\n", path, uid, gid);

    int ret = 0;

    _files_lock.lock();
    auto it = _files.find(path);
    if (it == _files.end()) {
        ret = -ENOENT;
    } else {
        file_entry_t fe = it->second;
        fe->st.st_uid = uid;
        fe->st.st_gid = gid;

        redis_sync(path, fe);
    }
    _files_lock.unlock();

    return ret;
}

static int _utimens(const char *path, const struct timespec tv[2]) {
    INFO("utimens [%s]\n", path);

    int ret = 0;

    _files_lock.lock();
    auto it = _files.find(path);
    if (it == _files.end()) {
        ret = -ENOENT;
    } else {
        file_entry_t fe = it->second;
        memcpy(&fe->st.st_atim, &tv[0], sizeof(struct timespec));
        memcpy(&fe->st.st_mtim, &tv[1], sizeof(struct timespec));
        redis_sync(path, fe);
    }
    _files_lock.unlock();

    return ret;
}

// file level apis

static int _truncate(const char *path, off_t size) {
    INFO("truncate file [%s] to %ld\n", path, size);
    int ret = 0;
    
    _files_lock.lock();
    auto it = _files.find(path);
    if (it == _files.end()) {
        ret = -ENOENT;
    } else {
        file_entry_t fe = it->second;

        if (size < fe->st.st_size) {
            cache_remove(fe->st.st_size - size);
        } else if (size > fe->st.st_size) {
            cache_add(size - fe->st.st_size);
        }
        fe->st.st_size = size;
        if (fe->fetched) {
            string data_path;
            get_data_path(data_path, _data_store_base, fe->link.c_str());
            if (truncate(data_path.c_str(), size)) {
                INFO("fail to truncate %s err=%d\n", path, errno);
            }
        }

        redis_sync(path, fe);

    }
    _files_lock.unlock();

    if (ret < 0) {
        DEBUG("truncate [%s] error %d\n", path, -ret);
    }
    
    return ret;
}

// open file with lock held
int _open_unsafe(const char *path, struct fuse_file_info *fi) {
    int ret = 0;
    file_entry_t fe;
    auto it = _files.find(path);
    if (it == _files.end()) {
        ret = -ENOENT;
    } else {
        // XXX PERM CHECK?
        fe = it->second;

        if (!S_ISREG(fe->st.st_mode)) {
            DEBUG("open a non-regular file [%s]\n", path);
            return -EINVAL;
        }

        // from now on we dont care about path
        
        string data_path;
        get_data_path(data_path, _data_store_base, fe->link.c_str());
        
      again:
        if (fe->fetched == 0) {
            grandet_fetch_task_t task = fe->fetch_task;
            int r = 0;
            if (task == NULL) {
                INFO("open start grandet_fetch_task link = [%s] size = %ld\n",
                     fe->link.c_str(), fe->st.st_size);
                cache_add(fe->st.st_size);
                r = grandet_fetch_task_start(path, fe->link.c_str(), data_path.c_str(), fe->st.st_size, &task);
                fe->fetch_task = task;
            } else
                grandet_fetch_task_get(task);

            fe_get(fe);
            
            if (r == 0) {                
                _files_lock.unlock();
                INFO("wait grandet_fetch_task {{\n");
                grandet_fetch_task_wait(task);
                INFO("wait grandet_fetch_task }}\n");
                _files_lock.lock();
            }

            grandet_fetch_task_put(task);
            
            // during the lock and unlock, the file entry may be
            // deleted, moved, or evicted. But it must be accessible
            // since we added a reference

            if (fe->deleted) {
                fe_put(fe);
                return -ENOENT;
            }

            // here fe is still in tree, so put the reference safely
            fe_put(fe);

            if (fe->fetch_task != task) {
                // do those check again to open the data link
                goto again;
            } else {
                // only the first one who put will get here, where a existing node
                // is just fetched from grandet
                fe->fetch_task = NULL;
                if (access(data_path.c_str(), F_OK) == 0) {
                    // test whether we failed
                    fe->fetched = 1;
                } else {
                    DEBUG("fetch failed, restore cache fetched = %d\n", fe->fetched);
                    ret = -ENOENT;
                    cache_remove(fe->st.st_size);
                }
            }
        }

        // when we reach here, the file entry may be just created,
        // fetched, moved (but not evicted), but at least it is a
        // existing regular file, and we know its data link.

        if (ret == 0) {
            int h = open(data_path.c_str(), fi->flags);
            if (h == -1) {
                ret = -errno;
            } else if (h >= FILE_DESC_UPPER_LIMIT) {
                DEBUG("open get a fd that exceeds upper limit\n");
                close(h);
                ret = -ENOMEM;
                h = -1;
            } else {
                struct stat st;
                fstat(h, &st);
                INFO("open get fd %d, len = %ld fe_len = %ld\n", h, st.st_size, fe->st.st_size);
                fi->fh = h;
                
                fd_fe[h] = fe;
                fd_touch_size[h] = 0;
                
                fe_get(fe);
                ++ fe->open_rc;
                if (fe->open_rc == 1) {
                    if (!list_empty(&fe->u_node)) {
                        INFO("remove %s from idle_list\n", fe->link.c_str());
                        list_del_init(&fe->u_node);
                    }
                }
            }
        }    
    }

    return ret;
}

static int _open(const char *path, struct fuse_file_info *fi) {
    fi->flags &= O_TRUNC | O_CREAT | 0x3;
    fi->fh = -1;
    INFO("open [%s], flags = %x\n", path, fi->flags);
    
    int ret;
    
    _files_lock.lock();
    ret = _open_unsafe(path, fi);
    _files_lock.unlock();

    if (ret < 0) {
        DEBUG("open [%s] failed\n", path);
    } else {
        // DEBUG("register fd %d [%s] %x\n", fi->fh, path, fi->flags);
    }
    return ret;
}

int
_create(const char *path, mode_t mode, struct fuse_file_info *fi) {
    fi->flags &= O_TRUNC | O_CREAT | 0x3;
    fi->fh = -1;
    INFO("create [%s], mode = %04o, flags = %x\n", path, mode, fi->flags);

    int ret = 0;
    string t_p, t_b;
    split_path(path, t_p, t_b);

    _files_lock.lock();
    map<string, file_entry_t>::iterator p_it, t_it;
    if ((p_it = _files.find(t_p)) == _files.end()) {
        ret = -ENOENT;
    } else if ((t_it = _files.find(path)) != _files.end()) {
        if (fi->flags & O_EXCL) {
            ret = -EEXIST;
        } else {
            ret = _open_unsafe(path, fi);
        }
    } else {
        // path is ready and target file does not exists
        file_entry_t p = p_it->second;

        boost::uuids::uuid key_uuid = boost::uuids::random_generator()();
        string data_key = boost::lexical_cast<string>(key_uuid);

        string data_path;
        get_data_path(data_path, _data_store_base, data_key.c_str());

        int h = open(data_path.c_str(), fi->flags, mode);
        if (h == -1) {
            ret = -errno;
        } else {
            if (ftruncate(h, 0)) {
                DEBUG("FAIL to ftruncate, err=%d\n", errno);
            }
            
            file_entry_t fe = new_fe();
        
            fe->name = t_b;
            fe->st.st_mode = S_IFREG | (mode & 0777);
            fe->st.st_size = 0;

            fe->parent = p;
            list_add(&p->c_list, &fe->c_node);
            _files[path] = fe;

            fe->link = data_key;
            INFO("create assign key [%s] to [%s]\n", fe->link.c_str(), path);
            
            if (h >= FILE_DESC_UPPER_LIMIT) {
                DEBUG("create exceed the max number of fd\n");
                close(h);
                h = -1;
                ret = -ENOMEM;
            } else {
                fi->fh = h;
                
                fd_fe[h] = fe;
                fd_touch_size[h] = 0;
                
                fe_get(fe);
            
                fe->fetched = 1;
                fe->content_dirty = 1;
                fe->open_rc = 1;

                _update_queue.add(fe);
            }

            redis_sync(path, fe);
        }
    }
    _files_lock.unlock();

    if (ret < 0) {
        DEBUG("create [%s] error %d\n", path, ret);
    } else {
        // DEBUG("register fd %d [%s] %x\n", fi->fh, path, fi->flags);
    }

    return ret;
}

static int _read(const char *path, char *buf, size_t size, off_t offset,
         struct fuse_file_info *fi) {
    int ret = pread(fi->fh, buf, size, offset);
    if (ret == -1) ret = -errno;
#ifdef DUMP_BUF
    dump_buf(buf, ret);
#endif
    return ret;
}

static int _write(const char *path, const char *buf, size_t size, off_t offset,
                  struct fuse_file_info *fi) {
    // while the file is open:
    //  it will not be in the idle list
    //  will be add to the sync queue at least once after the write
    int ret = pwrite(fi->fh, buf, size, offset);
    if (ret == -1)
        ret = -errno;
    else {
        file_entry_t fe = fd_fe[fi->fh];
        fe->content_dirty = 1;
        if (offset + size > fd_touch_size[fi->fh])
            fd_touch_size[fi->fh] = offset + size;
        if (fd_touch_size[fi->fh] > fe->st.st_size) {
            size_t delta = fd_touch_size[fi->fh] - fe->st.st_size;
            fe->st.st_size = fd_touch_size[fi->fh];
            cache_add(delta);
        }
    }
#ifdef DUMP_BUF
    dump_buf(buf, size);
#endif
    INFO("    touched size: %ld\n", fd_touch_size[fi->fh]);
    return ret;
}

static int _release(const char *path, struct fuse_file_info *fi) {
    INFO("release [%s]\n", path);

    if (fi->fh < 0 || fi->fh >= FILE_DESC_UPPER_LIMIT) {
        DEBUG("release invalid fd %lu, wtf\n", fi->fh);
        return -EINVAL;
    } else {
        // DEBUG("unregister fd %d\n", fi->fh);
    }
    
    _files_lock.lock();
    auto it = _files.find(path);
    file_entry_t fe;
    int mismatch = 1;
    
    if (it != _files.end()) {
        fe = it->second;
        if (fe->fetched > 0 && fe == fd_fe[fi->fh]) {
            mismatch = 0;
        
            _sync_fe_unsafe(path, fe, fi);
        
            -- fe->open_rc;
            INFO("  release: synced %s, open_rc=%d\n", path, fe->open_rc);
            fd_fe[fi->fh] = NULL;
            fd_touch_size[fi->fh] = 0;
            fe_put(fe);
            
            close(fi->fh);
            fi->fh = -1;
        }
    }

    if (mismatch) {
        if (fd_fe[fi->fh]) {
            fe = fd_fe[fi->fh];
            if (fe->open_rc <= 0) {
                DEBUG("release mismatch\n");
                check_fe(fe);
            } else if (fe->deleted) {
                // if it is removed, no more need to sync
                -- fe->open_rc;
                fd_fe[fi->fh] = NULL;
                fd_touch_size[fi->fh] = 0;
                fe_put(fe);

                close(fi->fh);
            } else {
                // moved to other place
                // sync with real path
                string real_path;
                if (get_real_path(fe, real_path) == 0) {
                    _sync_fe_unsafe(real_path.c_str(), fe, fi);
                } else {
                    DEBUG("get_real_path failed in release\n");
                }

                -- fe->open_rc;
                fd_fe[fi->fh] = NULL;
                fd_touch_size[fi->fh] = 0;
                fe_put(fe);
                
                close(fi->fh);
                fi->fh = -1;
            }

        } else {
            if (fcntl(fi->fh, F_GETFD) == -1) {
                DEBUG("release a NULL fe? the fd %lu is invalid\n", fi->fh);
            } else {
                DEBUG("release a NULL fe? the fd %lu is valid\n", fi->fh);
            }
        }
    }
#ifdef FLUSH_CACHE_ON_RELEASE
    cache_add(0);
#endif
    _files_lock.unlock();

    fi->fh = -1;
    return 0;
}

static int _fsync(const char *path, int isdatasync, struct fuse_file_info *fi) {
    INFO("fsync [%s]\n", path);

    (void) isdatasync;
    
    _files_lock.lock();
    if (fd_fe[fi->fh]) {
        string real_path;
        if (get_real_path(fd_fe[fi->fh], real_path) == 0) {
            _sync_fe_unsafe(real_path.c_str(), fd_fe[fi->fh], fi);
        }
    } else {
        DEBUG("fsync got a unmapped fd\n");
    }
    _files_lock.unlock();
    return 0;
}

static int _flush(const char *path, struct fuse_file_info *fi) {
    fdatasync(fi->fh);
}

static struct fuse_operations ops;

void init_fs() {
    list_init(&_idle_list);
    _cache_total_size_bytes = 0;
    
    _root = new_fe();
    _root->st.st_mode = S_IFDIR | 0777;
    _files["/"] = _root;

    // scan redis keys for filesystem metadata
    redisReply *reply;
    vector<string> file_keys;
    reply = (redisReply *)redisCommand(_redis_conn, "SCAN 0 MATCH f:*");
    while (true) {
        char *next = strdup(reply->element[0]->str);
        for (int i = 0; i < reply->element[1]->elements; ++ i) {
            file_keys.push_back(reply->element[1]->element[i]->str);
        }
        freeReplyObject(reply);
        if (strcmp(next, "0") == 0) {
            free(next);
            break;
        }
        reply = (redisReply *)redisCommand(_redis_conn, "SCAN %s MATCH f:*", next);
        free(next);
    }

    sort(file_keys.begin(), file_keys.end());

    // for (int i = 0; i < file_keys.size(); ++ i) {
    //     INFO("GET KEY: %s\n", file_keys[i].c_str());
    // }

    for (int i = 0; i < file_keys.size(); ++ i) {
        const string &k = file_keys[i];
               
        string path(k, 2, k.length() - 2);
        int slash_pos = path.length() - 1;
        while (slash_pos > 0 && path[slash_pos] != '/')
            -- slash_pos;
        if (slash_pos == path.length() - 1) continue;
        
        string base(path, 0, slash_pos == 0 ? 1 : slash_pos);
        string name(path, slash_pos + 1, path.length() - slash_pos - 1);

        INFO("init add [%s]/[%s]\n", base.c_str(), name.c_str());
        
        file_entry_t fe = new_fe();
        
        fe->name = name;
        redis_parse(path.c_str(), fe);

        fe->parent = _files[base];
        list_add(&fe->parent->c_list, &fe->c_node);
        _files[path] = fe;
    }
}

// xattr

static int _getxattr(const char *path, const char *name,
                     char *value, size_t size) {
    INFO("getxattr [%s] [%s]\n", path, name);

    int ret = 0;

    _files_lock.lock();
    auto it = _files.find(path);
    if (it == _files.end()) {
        ret = -ENOENT;
    } else {
        file_entry_t fe = it->second;
        auto eit = fe->xattr.find(name);
        if (eit == fe->xattr.end()) {
            ret = -ENOATTR;
        } else if (size == 0)  {
            ret = eit->second.length();
        } else if (size < eit->second.length()) {
            ret = -ERANGE;
        } else {
            memcpy(value, eit->second.c_str(), eit->second.length());
            ret = eit->second.length();
        }
    }
    _files_lock.unlock();

    return ret;
}

static int _setxattr(const char *path, const char *name,
                     const char *value, size_t size, int flags) {
    INFO("setxattr [%s] %s = %s\n", path, name, value);

    int ret = 0;

    _files_lock.lock();
    auto it = _files.find(path);
    if (it == _files.end()) {
        ret = -ENOENT;
    } else {
        file_entry_t fe = it->second;
        auto eit = fe->xattr.find(name);
        if ((flags & XATTR_CREATE) &&
            eit != fe->xattr.end()) {
            ret = -EEXIST;
        } else if ((flags & XATTR_REPLACE) &&
                   eit == fe->xattr.end()) {
            ret = -ENOATTR;
        }
        
        if (ret == 0) {
            fe->xattr[name] = string(value, size);
            redis_sync(path, fe);
        }
    }
    _files_lock.unlock();

    return ret;    
}

static int _removexattr(const char *path, const char *name) {
    INFO("removexattr [%s] %s\n", path, name);

    int ret = 0;

    _files_lock.lock();
    auto it = _files.find(path);
    if (it == _files.end()) {
        ret = -ENOENT;
    } else {
        file_entry_t fe = it->second;
        auto eit = fe->xattr.find(name);
        if (eit == fe->xattr.end()) {
            ret = -ENOATTR;
        } else {
            fe->xattr.erase(eit);
            redis_sync(path, fe);
        }
    }
    _files_lock.unlock();

    return ret;    
}

static int _listxattr(const char *path, char *list, size_t size) {
    INFO("listxattr [%s]\n", path);

    int ret = 0;

    _files_lock.lock();
    auto it = _files.find(path);
    if (it == _files.end()) {
        ret = -ENOENT;
    } else {
        file_entry_t fe = it->second;

        int rsize = 0;
        auto eit = fe->xattr.begin();
        while (eit != fe->xattr.end()) {
            rsize = rsize + eit->first.length() + 1;
            ++ eit;
        }

        if (size == 0) {
            ret = rsize;
        } else if (size < rsize) {
            return -ERANGE;
        } else {
            eit = fe->xattr.begin();
            char *ptr = list;
            while (eit != fe->xattr.end()) {
                memcpy(ptr, eit->first.c_str(), eit->first.length());
                ptr[eit->first.length()] = 0;
                ptr += eit->first.length() + 1;

                ++ eit;
            }
            ret = rsize;
        }
    }
    _files_lock.unlock();

    return ret;    
    
}

static void usage(const char *self) {
    printf("Usage %s [-d data_base] [-c cache_size_bytes] [-b bandwidth_requirement] [-l latency_requirement] [: fuse_options ...]\n"
           "\tDefaults:\n"
           "\t  data_base=./data\n"
           "\t  cache_size_bytes=1048576\n"
           "\t  bandwidth_requirement=1\n"
           "\t  latency_requirement=100\n", self);
}

int main(int argc, char **argv) {
    memset(&ops, 0, sizeof(ops));
    ops.getattr     = _getattr;
    ops.readdir     = _readdir;
    ops.readlink    = _readlink;
    ops.access      = _access;
    ops.mkdir       = _mkdir;
    ops.mknod       = _mknod;
    ops.unlink      = _unlink;
    ops.rmdir       = _rmdir;
    ops.symlink     = _symlink;
    ops.rename      = _rename;
    ops.chmod       = _chmod;
    ops.chown       = _chown;
    ops.utimens     = _utimens;
    ops.truncate    = _truncate;
    ops.open        = _open;
    ops.read        = _read;
    ops.write       = _write;
    ops.release     = _release;
    ops.create      = _create;
    ops.fsync       = _fsync;
    ops.flush       = _flush;
    ops.getxattr    = _getxattr;
    ops.setxattr    = _setxattr;
    ops.listxattr   = _listxattr;
    ops.removexattr = _removexattr;

    _data_store_base = "./data";
    _evicting_threshold_bytes = 1 << 20;

    int latency_req = 100;
    int band_req = 1;
    
    int cur = 1;
    while (cur < argc) {
        if (strcmp(argv[cur], "-d") == 0 && cur + 1 < argc) {
            _data_store_base = argv[cur + 1];
            cur += 2;
        } else if (strcmp(argv[cur], "-c") == 0 && cur + 1 < argc) {
            _evicting_threshold_bytes = atoi(argv[cur + 1]);
            cur += 2;
        } else if (strcmp(argv[cur], "-l") == 0 && cur + 1 < argc) {
            latency_req = atoi(argv[cur + 1]);
            cur += 2;
        } else if (strcmp(argv[cur], "-b") == 0 && cur + 1 < argc) {
            band_req = atoi(argv[cur + 1]);
            cur += 2;
        } else if (strcmp(argv[cur], "-h") == 0) {
            usage(argv[0]);
            exit(0);
        } else if (strcmp(argv[cur], ":") == 0) {
            break;
        } else {
            exit(1);
        }
    }

    argv[cur] = argv[0];

    // for (int i = cur; i < argc; ++ i) {
    //     printf("%s ", argv[i]);
    // }
    // printf("\n");

    grandet_init(_data_store_base.c_str(), latency_req, band_req);
    _redis_conn = redisConnect("127.0.0.1", 6379);

    pthread_create(&update_thread, NULL, &update_thread_func, NULL);
    
    init_fs();
    return fuse_main(argc - cur, argv + cur, &ops, NULL);
}
