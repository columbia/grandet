#include "dummyactor.h"
#include "key.h"
#include "value.h"
#include "profile.h"
using namespace std;

class RangeNotSupportedException : public exception {
};

void DummyActor::put(Key& key, shared_ptr<Value> value, const Profile& profile,
        const std::map<string, string>* metadata) {
    Bucket *bucket = getBucket(profile);
    {
        lock_guard<mutex> guard(bucket->lock);
        bucket->data[key.toString()] = value->toString();
        if (metadata) bucket->meta[key.toString()] = *metadata;
    }
}

shared_ptr<Value> DummyActor::get(Key& key, const Profile& profile,
        Range range, std::map<std::string, std::string>* metadata) {
    Bucket *bucket = getBucket(profile);
    Value* result = nullptr;
    {
        lock_guard<mutex> guard(bucket->lock);
        if (range) throw RangeNotSupportedException();
        if (bucket->data.find(key.toString()) != bucket->data.end()) {
            result = CreateStringValue(bucket->data[key.toString()]);
            if (metadata) {
                if (bucket->meta.find(key.toString()) != bucket->meta.end()) {
                    *metadata = bucket->meta[key.toString()];
                }
            }
        }
    }
    return shared_ptr<Value>(result);
}

void DummyActor::del(Key& key, const Profile& profile) {
    Bucket *bucket = getBucket(profile);
    {
        lock_guard<mutex> guard(bucket->lock);
        auto it = bucket->data.find(key.toString());
        if (it != bucket->data.end()) {
            bucket->data.erase(it);
            auto it2 = bucket->meta.find(key.toString());
            if (it2 != bucket->meta.end()) {
                bucket->meta.erase(it2);
            }
        }
    }
}

int64_t DummyActor::size(const Key& key, const Profile& profile) {
    Bucket *bucket = getBucket(profile);
    {
        lock_guard<mutex> guard(bucket->lock);
        auto it = bucket->data.find(key.toString());
        if (it != bucket->data.end()) {
            return it->second.size();
        } else {
            return 0;
        }
    }
}

string DummyActor::getName() {
    return "Dummy";
}

shared_ptr<Value> DummyActor::get_url(Key& key, time_t expiration,
        const Profile& profile) {
    return nullptr;
}

DummyActor::Bucket* DummyActor::getBucket(const Profile& profile) {
    string bucketName;
    if (!profile.get("bucket", &bucketName)) bucketName = "default";

    {
        lock_guard<mutex> guard(lock);
        return &data[bucketName];
    }
}
