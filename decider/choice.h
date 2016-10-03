#ifndef _CHOICE_H_
#define _CHOICE_H_

#include <string>
#include "profile.h"

struct Condition;
struct Actor;
class Config;

class Choice {
    int durability;
    int latency;
    int bandwidth;
    long minObjectSize;
    double costPerByte; // 10^-12$ per byte per month
    double costPerGet;  // 10^-12$ per req
    double costPerPut;  // 10^-12$ per req
    int id;
    std::string name;
    Actor *actor;
    Profile profile;

public:
    bool satisfy(const Condition& condition) const;
    double cost(const Condition& condition) const;
    double migrateCost(const Condition& condition, const Choice& current,
        bool extraGet) const;
    Actor* getActor() const;
    void setId(int id);
    int getId() const;
    const Profile& getProfile() const;
    std::string desc() const;
    std::string getName() const;
    double calcCost(int getCount, int putCount, long size) const;
    long realSize(long size) const;

    void load(Config *from, const std::string& path, bool statOnly);
    Choice(Actor* actor);
};

#endif // _CHOICE_H_
