#include <thread>
#include <mutex>
#include <cstdio>
#include <ctime>
#include <vector>
#include <iostream>
#include <sys/time.h>
#include <cassert>
#include <map>
#include <cmath>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <fcntl.h>
#include <unistd.h>

#include "grandet.h"

#include <libaws/aws.h>

#define VERIFY_RESULT 0

using namespace std;

const int key_size = 1;

int read_percent;
int size;
int req_count;

double total = 0;
mutex mtx;
bool not_finished = true;
int finished = 0;
int thread_count;
string path = "g";
double del_ratio = 0.05;

enum Ops {
    Read,
    Write
};

map<Ops, vector<long>> records;

long time_diff(const struct timespec& start, const struct timespec& end) {
    long sec_diff = end.tv_sec - start.tv_sec;
    long ns_diff = end.tv_nsec - start.tv_nsec;
    return sec_diff * 1000000000 + ns_diff;
}

void record_op(Ops op, const struct timespec& start, const struct timespec& end) {
    long time_us = time_diff(start, end);
    records[op].push_back(time_us);
}

void requestor_func(int id) {
    int count = 0;

    const string key(key_size, 'K');
    const string data(size, '#');

    string bucket = "cloud-econ-1-rr";

    char *access_key_id = getenv("AWS_ACCESS_KEY_ID");
    char *secret_access_key = getenv("AWS_SECRET_ACCESS_KEY");
    aws::S3ConnectionPtr conn = aws::AWSConnectionFactory::getInstance()->createS3Connection(access_key_id, secret_access_key);

    GrandetClient *client = CreateGrandetClient();
#if USE_TCP
    client->connect("127.0.0.1", "1481");
#else
    client->connect_unix("/tmp/grandet_socket");
#endif

//    client->put(key, data);

    struct timeval start_time;
    gettimeofday(&start_time, NULL);

    map<string, string> metadata;
    struct timespec req_start_time, req_end_time;
    while (count < req_count) {
        try {
        int op = rand() % 100;
        if (op < read_percent) {
//            gettimeofday(&req_start_time, NULL);

            clock_gettime(CLOCK_MONOTONIC, &req_start_time);
            if (path == "g") {
                const auto ret = client->get(key);
#if VERIFY_RESULT
                assert(ret.ok());
                if (ret.value != data) {
                    cout << ret.value << endl;
                    cout << data << endl;
                }
                assert(ret.value == data);
#endif
            } else if (path == "s3") {
                aws::GetResponsePtr response = conn->get(bucket, key);
                char buf[size];
                istream& value_stream = response->getInputStream();
                value_stream.read(buf, size);
            } else {
                char buf[size];
                ifstream inf(path + "/" + key);
                inf.read(buf, size);
                inf.close();
            }
//            gettimeofday(&req_end_time, NULL);
            clock_gettime(CLOCK_MONOTONIC, &req_end_time);
            record_op(Read, req_start_time, req_end_time);
        } else {
//            gettimeofday(&req_start_time, NULL);
            clock_gettime(CLOCK_MONOTONIC, &req_start_time);
            if (path == "g") {
                client->put(key, data);
            } else if (path == "s3") {
                istringstream s(data);
                conn->put(bucket, key, s, "binary/octet-stream", nullptr, -1, true);
            } else {
                int ouf = open((path + "/" + key).c_str(), O_WRONLY | O_CREAT, 0644);
                ftruncate(ouf, 0);
                write(ouf, data.c_str(), size);
                fdatasync(ouf);
                close(ouf);
            }
//            gettimeofday(&req_end_time, NULL);
            clock_gettime(CLOCK_MONOTONIC, &req_end_time);
            record_op(Write, req_start_time, req_end_time);
        }
        {
            unique_lock<mutex> lck(mutex);
            if (not_finished) {
                count++;
            } else {
                break;
            }
        }
        } catch (const std::exception& e) {
            cerr << "Exception! " << e.what() << endl;
        }
    }
    {
        unique_lock<mutex> lck(mutex);
        not_finished = false;
    }

    struct timeval end_time;
    gettimeofday(&end_time, NULL);

    double diff_us = end_time.tv_sec - start_time.tv_sec + (end_time.tv_usec - start_time.tv_usec) / 1000000.0;

    cout << "Thread " << id << "  count: " << count << " time: " << diff_us << " IOPS: " << count / diff_us << endl;

    {
        unique_lock<mutex> lck(mutex);
        total += count / diff_us;
    }

    delete client;
}

int main(int argc, char **argv) {
    if (argc > 1) {
        thread_count = atoi(argv[1]);
    } else {
        thread_count = 1;
    }

    if (argc > 2) {
        read_percent = atoi(argv[2]);
    } else {
        read_percent = 50;
    }

    if (argc > 3) {
        size = atoi(argv[3]);
    } else {
        size = 4096;
    }

    if (argc > 4) {
        req_count = atoi(argv[4]);
    } else {
        req_count = 1000;
    }

    if (argc > 5) {
        path = argv[5];
    }

    if (argc > 6) {
        del_ratio = atof(argv[6]);
    } else {
        del_ratio = 0.05;
    }

    cout << "Threads: " << thread_count << endl;
    cout << "Read %:  " << read_percent << endl;
    cout << "Size:    " << size << endl;
    cout << "Reqs:    " << req_count << endl;
    cout << "Path:    " << path << endl;
    cout << "Ratio:   " << (1 - del_ratio * 2) * 100 << endl;

    vector<thread> requestors;
    for (int i=0; i<thread_count; i++) {
        requestors.emplace_back(requestor_func, i);
    }

    for (int i=0; i<thread_count; i++) {
        requestors[i].join();
    }

    cout << "IOPS: " << total << endl;

    for (auto& entry : records) {
        switch (entry.first) {
            case Read:
                cout << "Read:" << endl;
                break;
            case Write:
                cout << "Write:" << endl;
                break;
        }
        long count = entry.second.size();
        double total = 0;

        vector<long>& queue = entry.second;
        sort(queue.begin(), queue.end());
        long removed = 0;
        for (int i=0; i<count * del_ratio; i++) {
            queue.erase(queue.begin());
            queue.pop_back();
            removed += 2;
        }
        count -= removed;

        for (auto record : entry.second) {
            total += (double)record / 1000;
        }
        double avg = total / count;
        double stddev = 0;
        for (auto record : entry.second) {
            stddev += ((double)record / 1000 - avg) * ((double)record / 1000 - avg);
//            cout << record << endl;
        }
        stddev = sqrt(stddev / (count - 1));
        cout << "count: " << count << " avg: " << avg << "  stddev: " << stddev << endl;
    }
}
