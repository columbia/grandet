#include "fsactor.h"
#include "actor.h"
#include "key.h"
#include "value.h"
#include "profile.h"
#include "log.h"
#include "config.h"

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/xattr.h>

#define ALIGNED_BUF

using namespace std;

class FSNoSSDPoolException : public std::exception {
    virtual const char* what() const noexcept override {
        return "No SSD pool configured in FS actor";
    }
};

class FSException : public std::exception {
    string msg;

    virtual const char* what() const noexcept override {
        return msg.c_str();
    }
public:
    FSException() {
        msg = string("FS error ") + to_string(errno);
    }
};

void FSActor::put(Key& key, shared_ptr<Value> value, const Profile& profile, const map<string, string>* metadata) {
    int rootDir = getRootDir(profile);
    int fd = openat(rootDir, key.toString().c_str(), O_WRONLY | O_CREAT, 0644);
    if (fd == -1) {
        throw FSException();
    }
    if (ftruncate(fd, 0) != 0) {
        throw FSException();
    }
    const string& sValue = value->toString();
    int wrote = write(fd, sValue.c_str(), sValue.size());
    if (wrote != sValue.size()) {
        close(fd);
        throw FSException();
    }
    fdatasync(fd);
    if (metadata) {
        for (const auto& meta : *metadata) {
            if (fsetxattr(fd, (string("user.") + meta.first).c_str(),
                    meta.second.c_str(), meta.second.size(), 0) < 0) {
                close(fd);
                throw FSException();
            }
        }
    }
    close(fd);
}

shared_ptr<Value> FSActor::get(Key& key, const Profile& profile, Range range, map<string, string>* metadata) {
    int rootDir = getRootDir(profile);
    int fd = openat(rootDir, key.toString().c_str(), O_RDONLY | (directMode ? O_DIRECT : 0));
    if (fd == -1) {
        throw FSException();
    }
    Value* result;
    if (range) {
        if (!range.start || !range.end) {
            close(fd);
            throw FSException();
        }
        lseek(fd, range.start.value(), SEEK_SET);
        off_t size = range.end.value() - range.start.value();
        string buf;
        buf.resize(size);
        int got = read(fd, &buf[0], size);
        buf.resize(got);
        result = CreateStringValue(buf);
    } else {
        off_t size = lseek(fd, 0, SEEK_END);
#ifdef ALIGNED_BUF
        char *buf;
        if (posix_memalign((void**)&buf, 4096, size) != 0) {
            throw FSException();
        }
#else
        string buf;
        buf.resize(size);
#endif
        lseek(fd, 0, SEEK_SET);
        int got = read(fd, &buf[0], size);
        if (got != size) {
            close(fd);
            throw FSException();
        }
#ifdef ALIGNED_BUF
        result = LootStringValue(string(buf, size));
        free(buf);
#else
        result = LootStringValue(buf);
#endif
    }
    if (metadata) {
        char* attrList;
        ssize_t xattrSize;
        while (true) {
            xattrSize = flistxattr(fd, NULL, 0);
            attrList = new char[xattrSize];
            ssize_t ret = flistxattr(fd, attrList, xattrSize);
            if (ret == -1) {
                Warn() << "flistxattr fail " << errno << endl;
                delete[] attrList;
                if (errno != ERANGE) {
                    close(fd);
                    throw FSException();
                } // ERANGE: try again
            } else {
                break;
            }
        }
        char *last = attrList;
        for (int i=0; i<xattrSize; i++) {
            if (attrList[i] == '\0') {
                string meta_key(last);
                ssize_t itemSize;
                char* item;
                while (true) {
                    itemSize = fgetxattr(fd, last, NULL, 0);
                    item = new char[itemSize];
                    ssize_t ret = fgetxattr(fd, last, item, itemSize);
                    if (ret == -1) {
                        Warn() << "fgetxattr fail " << errno << endl;
                        delete[] item;
                        if (errno != ERANGE) {
                            delete[] attrList;
                            close(fd);
                            throw FSException();
                        } // ERANGE: try again
                    } else {
                        break;
                    }
                }
                metadata->emplace(string(last).substr(5), string(item));
                delete[] item;
                last = &attrList[i+1];
            }
        }
        delete[] attrList;
    }
    close(fd);
    return shared_ptr<Value>(result);
}

shared_ptr<Value> FSActor::get_url(Key& key, time_t expiration, const Profile& profile) {
    return nullptr;
}

void FSActor::del(Key& key, const Profile& profile) {
    Dbg() << "Redis: deleting " << key.toString() << endl;
    int rootDir;
    string useSSD;
    if (profile.get("ssd", &useSSD) && useSSD == "true") {
        if (!ssdDir) {
            throw FSNoSSDPoolException();
        }
        rootDir = ssdDir;
    } else {
        rootDir = magDir;
    }
    unlinkat(rootDir, key.toString().c_str(), 0);
}

int64_t FSActor::size(const Key& key, const Profile& profile) {
    int rootDir = getRootDir(profile);
    struct stat st;
    if (fstatat(rootDir, key.toString().c_str(), &st, 0) != 0) {
        throw FSException();
    }
    return st.st_size;
}

string FSActor::getName() {
    return "FS";
}

FSActor::FSActor() {
    Config *config = GetConfigInstance();
    string magPath = config->get<string>("server.actors.FS.magPath");
    string ssdPath = config->get<string>("server.actors.FS.ssdPath");
    magDir = open(magPath.c_str(), O_RDONLY);
    if (magDir < 0) {
        Err() << "can't open magnetic path" << endl;
        throw FSException();
    }
    Dbg() << "magnetic FS at " << magPath << endl;
    if (ssdPath != "") {
        ssdDir = open(ssdPath.c_str(), O_RDONLY);
        if (ssdDir < 0) {
            Err() << "can't open SSD path" << endl;
            throw FSException();
        }
        Dbg() << "SSD FS at " << ssdPath << endl;
    } else {
        ssdDir = 0;
    }
    directMode = config->get<bool>("server.actors.FS.directMode", false);
    if (directMode) {
        Info() << "Using DIRECT mode in FS actor" << endl;
    } else {
        Info() << "Not using DIRECT mode in FS actor" << endl;
    }
}

FSActor::~FSActor() {
    if (ssdDir) close(ssdDir);
    close(magDir);
}

int FSActor::getRootDir(const Profile& profile) {
    string useSSD;
    if (profile.get("ssd", &useSSD) && useSSD == "true") {
        if (!ssdDir) {
            throw FSNoSSDPoolException();
        }
        return ssdDir;
    } else {
        return magDir;
    }
}
