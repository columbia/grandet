#ifndef _GRANDET_CONTROLLER_H_
#define _GRANDET_CONTROLLER_H_
#include <map>
#include <memory>
#include <string>
#include <mutex>

#include "choice.h"
#include "actor.h"

class Decider;
class Memory;
struct Estimator;
class Accountant;
class Statistic;

class LockMgr;
class ConnMgr;
class ConnInfo;
class Config;

class Key;
struct Value;
struct Requirements;
class Condition;

class Controller {
    typedef int choice_id;
    std::map<choice_id, Choice> choices;
    std::map<std::string, std::unique_ptr<Actor>> actors;

    std::unique_ptr<Decider> decider;
    std::unique_ptr<Estimator> estimator;
    std::unique_ptr<Memory> memory;
    std::unique_ptr<Accountant> accountant;
    std::unique_ptr<Statistic> stat;

    std::unique_ptr<LockMgr> lockMgr;
    std::unique_ptr<ConnMgr> connMgr;
    Config* config;
    bool enableGetMigration;
    bool enableStat;

    struct KeyInfo {
        std::unique_ptr<Requirements> requirements;
        size_t size;
    };

    std::mutex infoLock;
    std::map<Key, KeyInfo> info;

    void loadChoices(Config *from, bool statOnly);
    void loadActors();

    Condition prepareCondition(Key& key, size_t size,
            const Requirements& requirements);
    void checkMigration(Key& key, const Choice& current, bool extraGet,
            std::shared_ptr<Value> valueGot);
    void storeInto(Key& key, std::shared_ptr<Value> value,
            const Choice& choice, const Requirements& requirements);
    void deleteFrom(Key& key, const Choice& choice);
public:
    // Handle different operations
    void handlePut(Key& key, std::shared_ptr<Value> value,
            const Requirements& requirements);
    std::shared_ptr<Value> handleGet(Key& key,
            const Requirements& requirements);
    void handleDel(Key& key,
            const Requirements& requirements);
    int64_t handleSize(Key& key,
            const Requirements& requirements);
    // Dump information of all keys
    std::string handleKeys();
    void handleReset();
    std::string handleDist();
    void handleAccount();

    // Lock/Unlock a key
    // #return true: got lock false: need wait
    bool handleLock(const Key& key, ConnInfo *conn);
    // #return null: no waiter
    ConnInfo* handleUnlock(const Key& key);

    // Load choices from initial config
    void loadDefaultChoices();
    // Update choices from profiling data
    void updateChoices(const std::string& filename);
    void start();
    Choice& getChoice(choice_id cid);
    const std::map<choice_id, Choice>& getChoices();

    Controller();
    ~Controller();
};

#endif // _GRANDET_CONTROLLER_H_
