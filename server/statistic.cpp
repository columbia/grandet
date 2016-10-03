#include "key.h"
#include "statistic.h"
using namespace std;

void Statistic::notifyGet(const Key& key) {
    string k = key.toString();
    data[k].get++;
}

void Statistic::notifyPut(const Key& key) {
    string k = key.toString();
    data[k].put++;
}

void Statistic::getInfo(const Key& key, long& get, long& put) {
    string k = key.toString();
    get = data[k].get;
    put = data[k].put;
}

void Statistic::reset() {
    data.clear();
}
