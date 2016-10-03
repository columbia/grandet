#include <thread>
#include <mutex>
#include <cstdio>
#include <ctime>
#include <vector>
#include <iostream>
#include <sys/time.h>
#include <cassert>
#include "grandet.h"

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

char key_char(int id) {
    if (id < 26) return 'a' + id;
    if (id < 52) return 'A' + id - 26;
    if (id < 62) return '0' + id - 52;
    id -= 62;
    switch (id) {
        case 0: return '#';
        case 1: return '%';
        case 2: return '+';
        case 3: return '-';
        case 4: return ':';
        case 5: return '=';
    }
    abort();
}

string generate_key(int key_size, int id) {
    string element;
    if (id < 64) {
        return string(key_size, key_char(id));
    } else {
        do {
            element += key_char(id % 64);
            id /= 64;
        } while (id != 0);
        return string(element, key_size);
    }
}

void requestor_func(int id) {
    int count = 0;

    const string key = generate_key(key_size, id);
    const string data(size, '#');

    GrandetClient *client = CreateGrandetClient();
#if USE_TCP
    client->connect("127.0.0.1", "1481");
#else
    client->connect_unix("/tmp/grandet_socket");
#endif

    client->put(key, data);

    struct timeval start_time;
    gettimeofday(&start_time, NULL);

    while (count < req_count) {
        int op = rand() % 100;
        if (op < read_percent) {
            const auto ret = client->get(key);
            assert(ret.ok());
#if VERIFY_RESULT
            if (ret.value != data) {
                cout << ret.value << endl;
                cout << data << endl;
            }
            assert(ret.value == data);
#endif
        } else {
            client->put(key, data);
        }
        {
            unique_lock<mutex> lck(mutex);
            if (not_finished) {
                count++;
            } else {
                break;
            }
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

    cout << "Threads: " << thread_count << endl;
    cout << "Read %:  " << read_percent << endl;
    cout << "Size:    " << size << endl;
    cout << "Reqs:    " << req_count << endl;

    vector<thread> requestors;
    for (int i=0; i<thread_count; i++) {
        requestors.emplace_back(requestor_func, i);
    }

    for (int i=0; i<thread_count; i++) {
        requestors[i].join();
    }

    cout << "IOPS: " << total << endl;
}
