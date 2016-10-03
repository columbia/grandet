#include "s3actor.h"

#include <boost/algorithm/string/trim.hpp>
#include <cstdlib>
#include <fstream>
#include <map>
#include <sstream>
#include <string>

#include "log.h"
#include "key.h"
#include "profile.h"
#include "value.h"
#include "config.h"

using namespace std;

const int GET_BUFFER_SIZE = 4096;

void S3Actor::put(Key& key, shared_ptr<Value> value, const Profile& profile, const map<string, string>* metadata) {
    PoolConn<aws::S3ConnectionPtr> conn(&pool);
    string bucket;
    profile.get("bucket", &bucket);
    string rr;
    bool isRR = profile.get("reducedRedundancy", &rr) && rr == "true";
    if (value->hasFilename()) {
        ifstream s(value->toFilename());
        if (s.is_open()) {
            conn->put(bucket, key.toString(), s, "binary/octet-stream", metadata, -1, isRR);
        } else {
            Err() << "Error opening file " << value->toFilename() << endl;
        }
    } else {
        istringstream s(value->toString());
        conn->put(bucket, key.toString(), s, "binary/octet-stream", metadata, -1, isRR);
    }
}

shared_ptr<Value> S3Actor::get(Key& key, const Profile& profile, Range range, map<string, string>* metadata) {
    PoolConn<aws::S3ConnectionPtr> conn(&pool);
    string bucket;
    profile.get("bucket", &bucket);
    aws::GetResponsePtr response;
    if (range) {
        string ranges_specifier{"bytes="};
        int64_t start = range.start.value_or(0);
        ranges_specifier += to_string(start);
        if (start >= 0) {
            ranges_specifier += '-';
            if (range.end) {
                ranges_specifier += to_string(*range.end);
            }
        }
        map<string, string> headers;
        headers.insert(make_pair("Range", ranges_specifier));
        response = conn->get(bucket, key.toString(), &headers);
    } else {
        response = conn->get(bucket, key.toString());
    }
    if (metadata) {
        for (const auto& meta : response->getMetaData()) {
            metadata->emplace(meta.first, boost::algorithm::trim_right_copy(meta.second));
        }
    }
    istream& value_stream = response->getInputStream();

#ifdef USE_STREAMBUF
    istreambuf_iterator<char> eos;
    string ret(istreambuf_iterator<char>(value_stream), eos);
#else
    string ret;
    ret.resize(response->getContentLength());
    value_stream.read(&ret[0], response->getContentLength());
#endif
    return shared_ptr<Value>(LootStringValue(ret));
}

shared_ptr<Value> S3Actor::get_url(Key& key, time_t expiration, const Profile& profile) {
    PoolConn<aws::S3ConnectionPtr> conn(&pool);
    string bucket;
    profile.get("bucket", &bucket);
    return shared_ptr<Value>(CreateUrlValue(conn->getQueryString(bucket, key.toString(), expiration)));
}

void S3Actor::del(Key& key, const Profile& profile) {
    PoolConn<aws::S3ConnectionPtr> conn(&pool);
    string bucket;
    profile.get("bucket", &bucket);
    Dbg() << "S3: deleting " << key.toString() << endl;
    conn->del(bucket, key.toString());
}

int64_t S3Actor::size(const Key& key, const Profile& profile) {
    PoolConn<aws::S3ConnectionPtr> conn(&pool);
    string bucket;
    profile.get("bucket", &bucket);
    return conn->head(bucket, key.toString())->getContentLength();
}

string S3Actor::getName() {
    return "S3";
}

S3Actor::S3Actor() {
    char *access_key_id = getenv("AWS_ACCESS_KEY_ID");
    char *secret_access_key = getenv("AWS_SECRET_ACCESS_KEY");

    if (!access_key_id || !secret_access_key) {
        Log() << "Missing AWS_ACCESS_KEY_ID or AWS_SECRET_ACCESS_KEY." << endl;
        exit(1);
    }

    for (int i=0; i<GetConfigInstance()->get<int>("server.actors.S3.conns", 10); i++) {
        pool.put(aws::AWSConnectionFactory::getInstance()->createS3Connection(
                    access_key_id, secret_access_key));
    }
}
