#ifndef _DECIDER_H_
#define _DECIDER_H_

#include "choice.h"

#include <vector>

struct Condition;

class Decider {
    std::vector<Choice> choices;

public:
    // Choose a choice according to the condition
    // #return
    //   null: no satisfying choice
    //   non-null: the choice chosen
    Choice* choose(const Condition& condition, double* cost);
    Choice* findBetter(const Condition& condition, const Choice& current,
        bool extraGet, double* cost);
    void addChoice(const Choice& choice);
    void clear();
};

#endif // _DECIDER_H_
