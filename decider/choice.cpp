#include <algorithm>
#include <sstream>

#include "choice.h"
#include "condition.h"
#include "config.h"
#include "actor.h"
#include "log.h"

using namespace std;

bool Choice::satisfy(const Condition& condition) const {
    return (condition.durabilityRequired <= durability
            && condition.latencyRequired >= latency
            && condition.bandwidthRequired <= bandwidth);
}

double Choice::cost(const Condition& condition) const {
    Dbg() << "stor: " << max(condition.size, minObjectSize) * costPerByte
        * condition.timeSpan / 86400 / 30 << endl;
    Dbg() << "get:  " << condition.getRequests * costPerGet << endl;
    Dbg() << "put:  " << condition.putRequests * costPerPut << endl;
    double total = max(condition.size, minObjectSize) * costPerByte * condition.timeSpan / 86400 / 30
        + condition.getRequests * costPerGet + condition.putRequests * costPerPut;
    Dbg() << "total:  " << total << endl;
    return total;
}

Actor* Choice::getActor() const {
    return actor;
}

Choice::Choice(Actor* actor) : actor(actor) {
}

void Choice::setId(int id) {
    this->id = id;
}

int Choice::getId() const {
    return id;
}

const Profile& Choice::getProfile() const {
    return profile;
}

void Choice::load(Config *config, const string& path, bool statOnly) {
    name = config->get<string>(path + "name");
    Dbg() << "loading choice " << name << endl;
    id = config->get<int>(path + "id");
    durability = config->get<int>(path + "durability");
    latency = config->get<int>(path + "latency");
    bandwidth = config->get<int>(path + "bandwidth");
    minObjectSize = config->get<long>(path + "minObjectSize");
    costPerByte = config->get<double>(path + "costPerByte");
    costPerGet = config->get<double>(path + "costPerGet");
    costPerPut = config->get<double>(path + "costPerPut");
    if (!statOnly) {
        profile.load(config, path);
    }
}

string Choice::desc() const {
    stringstream ss;
    ss << name << "(" << actor->getName() << "," << profile.desc() << ")";
    return ss.str();
}

string Choice::getName() const { return name; }

double Choice::calcCost(int getCount, int putCount, long size) const {
    if (size == 0) {
        return getCount * costPerGet + putCount * costPerPut;
    } else {
        return getCount * costPerGet + putCount * costPerPut + max(size, minObjectSize) * costPerByte;
    }
}

double Choice::migrateCost(const Condition& condition, const Choice& current,
        bool extraGet) const {
    double migCost = cost(condition) +
        + costPerPut * condition.timeSpan / 86400 / 30
        + (extraGet ? current.costPerGet * condition.timeSpan / 86400 / 30 : 0);
    Dbg() << name << " migCost:  " << migCost << endl;
    return migCost;
}

long Choice::realSize(long size) const {
    return max(size, minObjectSize);
}
