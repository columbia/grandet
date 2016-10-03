#include "accountant.h"
#include "log.h"
#include "choice.h"
#include "controller.h"

using namespace std;

void Accountant::recordGet(choice_id cid) {
    lock_guard<mutex> guard(statLock);
    stats[cid].getCount++;

    Choice& choice = master->getChoice(cid);
    totalCost += choice.calcCost(1, 0, 0);
}

void Accountant::recordPut(choice_id cid, size_t size) {
    lock_guard<mutex> guard(statLock);
    stats[cid].putCount++;

    Choice& choice = master->getChoice(cid);
    totalCost += choice.calcCost(0, 1, 0);
    stats[cid].putSize += choice.realSize(size);
    stats[cid].dataSize += size;
}

void Accountant::recordDel(choice_id cid, size_t size) {
    lock_guard<mutex> guard(statLock);
    stats[cid].putCount++;

    Choice& choice = master->getChoice(cid);
    stats[cid].putSize -= choice.realSize(size);
    stats[cid].dataSize -= size;
}

void Accountant::recordMigrate(choice_id oldId, choice_id newId, size_t size,
        bool extraGet) {
    lock_guard<mutex> guard(statLock);
    if (extraGet) stats[oldId].getCount++;
    stats[newId].putCount++;

    migPut++;
    if (extraGet) migGet++;

    Choice& oldChoice = master->getChoice(oldId);
    Choice& newChoice = master->getChoice(newId);
    stats[oldId].putSize -= oldChoice.realSize(size);
    stats[newId].putSize += oldChoice.realSize(size);
    stats[oldId].dataSize -= size;
    stats[newId].dataSize += size;

    double thisMigCost = newChoice.calcCost(0, 1, 0) +
        (extraGet ? oldChoice.calcCost(1, 0, 0) : 0);
    migCost += thisMigCost;
    totalCost += thisMigCost;
}

void Accountant::printInfo() {
    Info() << "=========== Accountant report ================" << endl;
    Info() << "Estimated cost so far: " <<
        (cost * 60 * 24 * 30 / 1000) << " nano$" << endl;
    double storeCost = 0;
    {
        lock_guard<mutex> guard(statLock);
        long tPut = -migPut, tGet = -migGet, tSize = 0;
        for (const auto& it : stats) {
            choice_id cid = it.first;

            tPut += stats[cid].putCount;
            tGet += stats[cid].getCount;
            tSize += stats[cid].putSize;
        }
        for (const auto& it : master->getChoices()) {
            choice_id cid = it.first;
            const Choice& choice = it.second;
            Info() << "Choice " << choice.desc() << ":" << endl;
            if (stats.find(cid) != stats.end()) {
                Info() << "GET: " << stats[cid].getCount << endl;
                Info() << "PUT: " << stats[cid].putCount << " ("
                    << stats[cid].putSize << "," << stats[cid].dataSize
                    << ")" << endl;

                Info() << "COST: " << (choice.calcCost(stats[cid].getCount,
                            stats[cid].putCount, stats[cid].putSize) / 1000) << " nano$" << endl;
                storeCost += choice.calcCost(0, 0, stats[cid].putSize);
            }
//            Info() << "SOLO: " << (choice.calcCost(tGet, tPut, tSize) / 1000) << " nano$" << endl;
        }
    }
    Info() << "Real cost: " << (totalCost + storeCost) / 1000 << " nano$" << endl;
    Info() << "Storage cost: " << storeCost / 1000 << " nano$" << endl;
    Info() << "Migration cost: " << migCost / 1000 << " nano$" << endl;
    Info() << "=========== Accountant report end ============" << endl;
}

void Accountant::recordEstimate(double estimatedCost) {
    cost += estimatedCost;
}

Accountant::Accountant(Controller* master) : master(master), cost(0), totalCost(0), migPut(0), migGet(0) {}

void Accountant::reset() {
    lock_guard<mutex> guard(statLock);

    stats.clear();
    cost = 0;
    totalCost = 0;
    migPut = 0;
    migGet = 0;
    migCost = 0;
}
