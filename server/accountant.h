#ifndef _GRANDET_ACCOUNTANT_H_
#define _GRANDET_ACCOUNTANT_H_

#include <map>
#include <mutex>

class Controller;

class Accountant {
    typedef int choice_id;
    struct ChoiceStat {
        int getCount;
        int putCount;
        long putSize;  // cost-side
        long dataSize; // data-side
    };

    std::map<choice_id, ChoiceStat> stats;
    std::mutex statLock;

    double cost;
    double totalCost;
    double migCost;
    int migPut;
    int migGet;
    Controller *master;
public:
    void recordGet(choice_id cid);
    void recordPut(choice_id cid, size_t size);
    void recordDel(choice_id cid, size_t size);
    void recordMigrate(choice_id oldId, choice_id newId, size_t size,
            bool extraGet);
    void recordEstimate(double estimatedCost);
    void printInfo();
    void reset();
    Accountant(Controller* master);
};

#endif //  _GRANDET_ACCOUNTANT_H_
