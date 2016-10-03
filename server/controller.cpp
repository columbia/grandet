#include "controller.h"
#include "key.h"
#include "value.h"
#include "redisactor.h"
#include "s3actor.h"
#include "dummyactor.h"
#include "fsactor.h"
#include "choice.h"
#include "requirements.h"
#include "condition.h"
#include "estimator.h"
#include "decider.h"
#include "memory.h"
#include "lockmgr.h"
#include "tcpconnmgr.h"
#include "unixconnmgr.h"
#include "accountant.h"
#include "log.h"
#include "config.h"
#include "statistic.h"
using namespace std;

const int DEFAULT_SERVER_PORT = 1481;
const int EST_INTERVAL = 60;

class NoChoiceAvailableException : public exception {
};

Condition Controller::prepareCondition(Key& key, size_t size,
        const Requirements& requirements) {
    Condition condition;
    condition.size = size;
    condition.durabilityRequired = requirements.durabilityRequired;
    condition.latencyRequired = requirements.latencyRequired;
    condition.bandwidthRequired = requirements.bandwidthRequired;
    condition.getRequests = estimator->estimateGet(key, requirements, EST_INTERVAL);
    condition.putRequests = estimator->estimatePut(key, requirements, EST_INTERVAL);
    condition.timeSpan = EST_INTERVAL;
    return condition;
}

void Controller::storeInto(Key& key, shared_ptr<Value> value,
        const Choice& choice, const Requirements& requirements) {
    Actor* actor = choice.getActor();
    Dbg() << "chose " << choice.desc() << " for this" << endl;
    if (requirements.metadata.empty()) {
        actor->put(key, value, choice.getProfile());
    } else {
        actor->put(key, value, choice.getProfile(), &requirements.metadata);
    }
}

void Controller::handlePut(Key& key, shared_ptr<Value> value,
        const Requirements& requirements) {
    Condition condition = prepareCondition(key, value->getSize(), requirements);
    {
        lock_guard<mutex> gurad(infoLock);
        info[key] = KeyInfo{make_unique<Requirements>(requirements),
            value->getSize()};
    }
    double estimatedCost;
    Choice* choice = decider->choose(condition, &estimatedCost);
    if (choice == nullptr) {
        throw NoChoiceAvailableException();
    }
    accountant->recordEstimate(estimatedCost);
    accountant->recordPut(choice->getId(), value->getSize());
    storeInto(key, value, *choice, requirements);
    choice_id old_choice_id = memory->remember(key, choice->getId());
    if (old_choice_id != 0) {
        auto it = choices.find(old_choice_id);
        assert(it != choices.end());
        Choice& old_choice = it->second;
        old_choice.getActor()->del(key, old_choice.getProfile());
    }
    estimator->notifyPut(key, requirements);
    if (enableStat) {
        stat->notifyPut(key);
    }
}

shared_ptr<Value> Controller::handleGet(Key& key, const Requirements& requirements) {
    shared_ptr<Value> value;

    Requirements mutable_requirements = requirements;

    choice_id cid;
    if (memory->recall(key, &cid)) {
        auto cit = choices.find(cid);
        assert(cit != choices.end());
        Choice choice = cit->second;
        accountant->recordGet(cid);

        Actor* actor = choice.getActor();
        Dbg() << "choice " << choice.desc() << "handle this" << endl;
        if (requirements.prefer_url) {
            value = actor->get_url(key, requirements.expiration, choice.getProfile());
        }
        if (!value) {
            value = actor->get(key, choice.getProfile(), requirements.range, &mutable_requirements.metadata);
        }

        if (!value) {
            Err() << "cannot find " << key.toString() << " in choice " << choice.desc() << endl;
            for (auto& choice_pair : choices) {
                Choice& choice = choice_pair.second;
                if (choice.getActor()->get(key, choice.getProfile(), requirements.range, &mutable_requirements.metadata)) {
                    Err() << "found key in " << choice.desc() << "instead" << endl;
                    abort();
                }
            }
        }

        if (enableGetMigration) {
            bool extraGet = !value->hasUrl();
            checkMigration(key, choice, extraGet, value);
        }
    }

    estimator->notifyGet(key, mutable_requirements);
    if (enableStat) {
        stat->notifyGet(key);
    }

    return value;
}

void Controller::checkMigration(Key& key, const Choice& current,
        bool extraGet, shared_ptr<Value> valueGot) {
    Requirements requirements;
    size_t size;
    {
        lock_guard<mutex> guard(infoLock);
        if (info.count(key) == 0) return;
        requirements = *info[key].requirements.get();
        size = info[key].size;
    }
    Condition condition = prepareCondition(key, size, requirements);
    double estimatedCost;
    Choice* choice = decider->findBetter(condition, current, extraGet,
            &estimatedCost);
    if (choice != nullptr) {
        Dbg() << "migrating from " << current.desc() << " to " << choice->desc() << endl;
        shared_ptr<Value> value;
        Requirements migrateRequirements;
        if (extraGet) {
            value = current.getActor()->get(key, current.getProfile());
        } else {
            value = valueGot;
        }
        storeInto(key, value, *choice, requirements);
        choice_id old_id = memory->remember(key, choice->getId());
        deleteFrom(key, current);
        accountant->recordMigrate(current.getId(), choice->getId(), size,
                extraGet);
    }
}

void Controller::deleteFrom(Key& key, const Choice& choice) {
    Actor* actor = choice.getActor();
    actor->del(key, choice.getProfile());
}

void Controller::handleDel(Key& key, const Requirements& requirements) {
    // Maybe later
//    estimator->notifyDel(key, requirements);
    choice_id cid;
    if (memory->recall(key, &cid)) {
        auto cit = choices.find(cid);
        assert(cit != choices.end());
        deleteFrom(key, cit->second);
        size_t size;
        {
            lock_guard<mutex> guard(infoLock);
            if (info.count(key) == 0) {
                size = 0;
            } else {
                size = info[key].size;
            }
        }
        accountant->recordDel(cid, size);

        memory->forget(key);
    }
}

int64_t Controller::handleSize(Key& key, const Requirements& requirements) {
    // TODO: estimator->notifySize(key, requirements);

    choice_id cid;
    if (memory->recall(key, &cid)) {
        auto cit = choices.find(cid);
        assert(cit != choices.end());
        Choice& choice = cit->second;

        Actor* actor = choice.getActor();
        return actor->size(key, choice.getProfile());
    }
    return -1;
}

string Controller::handleKeys() {
    accountant->printInfo();

    stringstream ss;
    const map<Key, choice_id>& mem = memory->dump();
    for (const auto& it : mem) {
        string key = it.first.toString();
        choice_id choice_id = it.second;
        Choice& choice = choices.find(choice_id)->second;

        string flags;
        if (lockMgr->isLocked(it.first)) {
            flags = "L";
        } else {
            flags = "_";
        }

        ss << key << " " << flags << " " << choice.desc() << endl;
    }
    return ss.str();
}

bool Controller::handleLock(const Key& key, ConnInfo* conn) {
    return lockMgr->lock(key, conn);
}

ConnInfo* Controller::handleUnlock(const Key& key) {
    return lockMgr->unlock(key);
}

Controller::Controller() {
    config = GetConfigInstance();
    decider = make_unique<Decider>();
    if (config->get<string>("server.unixaddress", "") != "") {
        connMgr = make_unique<UNIXConnMgr>(this,
                config->get<string>("server.unixaddress"));
    } else {
#if USE_ASIO
        connMgr = make_unique<TCPConnMgr>(this,
                config->get<int>("server.port", DEFAULT_SERVER_PORT));
#endif
    }
    estimator = unique_ptr<Estimator>(GetEstimator());
    memory = make_unique<Memory>();
    lockMgr = make_unique<LockMgr>();
    accountant = make_unique<Accountant>(this);
    stat = make_unique<Statistic>();
    loadActors();

    enableGetMigration = config->get<bool>("server.getMig", true);
    if (enableGetMigration) {
        Info() << "Migration-on-get enabled" << endl;
    }
    enableStat = config->get<bool>("server.stat", false);
    if (enableStat) {
        Info() << "Collecting stat info" << endl;
    }
}

void Controller::loadDefaultChoices() {
    loadChoices(config, false);
}

void Controller::updateChoices(const std::string& filename) {
    unique_ptr<Config> newInfo = make_unique<Config>();
    newInfo->load(filename);
    loadChoices(newInfo.get(), true);
}

void Controller::loadActors() {
    actors.clear();
    int begin = 0;
    string actorList = config->get<string>("server.actor", "Redis,S3,Dummy,FS") + ",";
    for (int i=0; i<actorList.size(); i++) {
        if (actorList[i] == ',') {
            string actorName = actorList.substr(begin, i - begin);
            if (actorName != "") {
                if (actorName == "Redis") {
                    actors.insert(make_pair(actorName, make_unique<RedisActor>()));
                } else if (actorName == "S3") {
                    actors.insert(make_pair(actorName, make_unique<S3Actor>()));
                } else if (actorName == "Dummy") {
                    actors.insert(make_pair(actorName, make_unique<DummyActor>()));
                } else if (actorName == "FS") {
                    actors.insert(make_pair(actorName, make_unique<FSActor>()));
                }
            }
            begin = i + 1;
        }
    }
}

void Controller::loadChoices(Config *from, bool statOnly) {
    if (!statOnly) {
        choices.clear();
    }
    decider->clear();
    int choiceCount = from->getSize("server.choices");
    for (int i=0; i<choiceCount; i++) {
        string path = string("server.choices.") + to_string(i) + ".";
        if (!statOnly) {
            string sActor = from->get<string>(path + "actor");
            Actor *actor = actors[sActor].get();
            if (!actor) {
                Log() << "config error: unknown actor " << sActor << endl;
                continue;
            }

            Choice choice(actor);
            choice.load(from, path, statOnly);

            choices.insert(make_pair(choice.getId(), choice));
            decider->addChoice(choice);
        } else {
            choice_id cid = from->get<int>(path + "id");
            const auto& it = choices.find(cid);
            Choice& choice = it->second;
            choice.load(from, path, statOnly);
            decider->addChoice(choice);
        }
    }
}

void Controller::start() {
    memory->load();
    connMgr->start();
}

Controller::~Controller() {
}

Choice& Controller::getChoice(choice_id cid) {
    return choices.find(cid)->second;
}

const map<Controller::choice_id, Choice>& Controller::getChoices() {
    return choices;
}

void Controller::handleReset() {
    Requirements requirements;
    map<Key, choice_id> mem = memory->dump();
    for (auto& it : mem) {
        Key key = it.first;
        try {
            handleDel(key, requirements);
        } catch (const exception& e) {
            Err() << "Delete " << key.toString() << " fail: " << e.what() << endl;
        }
    }
    estimator->reset();
    accountant->reset();
    lockMgr->reset();
    stat->reset();
    info.clear();
    Info() << "Server reset finished." << endl;
}

string Controller::handleDist() {
    stringstream ss;
    const map<Key, choice_id>& mem = memory->dump();
    double tcost = 0;
    if (enableStat) Info() << "======== Cost Report ========" << endl;
    try {
        memory->lockIt();
        for (auto& it : mem) {
            string key = it.first.toString();
            choice_id choice_id = it.second;
            Choice& choice = choices.find(choice_id)->second;

            if (!(info.count(key) == 1)) {
                continue;
            }
            size_t size;
            Requirements requirements;
            {
                lock_guard<mutex> guard(infoLock);
                requirements = *info[key].requirements.get();
                size = info[key].size;
            }

            ss << key << " " << size << " " << choice.desc();

            if (enableStat) {
                double cost = 0;
                long get, put;
                stat->getInfo(key, get, put);

                Condition condition;
                condition.size = size;
                condition.durabilityRequired = requirements.durabilityRequired;
                condition.latencyRequired = requirements.latencyRequired;
                condition.bandwidthRequired = requirements.bandwidthRequired;
                condition.getRequests = get;
                condition.putRequests = put;
                condition.timeSpan = 86400 * 30;
                Choice *bestChoice = decider->choose(condition, &cost);
                if (bestChoice == nullptr) {
                    Err() << "can't find optimal place?" << endl;
                } else {
                    tcost += cost;
                }
                ss << " " << get << " " << put << " " << bestChoice->desc();
            }
            ss << endl;
        }
        if (enableStat) {
            for (const auto& it : getChoices()) {
                double myCost = 0;
                double realMyCost = 0;
                const Choice& myChoice = it.second;
                for (auto& it : mem) {
                    string key = it.first.toString();
                    choice_id choice_id = it.second;
                    Choice& choice = choices.find(choice_id)->second;

                    if (!(info.count(key) == 1)) {
                        continue;
                    }

                    size_t size;
                    {
                        lock_guard<mutex> guard(infoLock);
                        size = info[key].size;
                    }

                    long get, put;
                    stat->getInfo(key, get, put);

                    double cost = myChoice.calcCost(get, put, size);
                    myCost += cost;
                    if (choice_id == myChoice.getId()) realMyCost += cost;
                }
                Info() << myChoice.getName() << " SOLO: " << myCost / 1000 << " nano$"
                    << "  COST: " << realMyCost / 1000 << " nano$" << endl;
            }
        }
        memory->unlockIt();
        if (enableStat) {
            Info() << "Optimal total cost: " << tcost / 1000 << " nano$" << endl;
        }
    } catch (...) {
        Err() << "got exception in handleDist" << endl;
        memory->unlockIt();
    }
    return ss.str();
}

void Controller::handleAccount() {
    accountant->printInfo();
}
