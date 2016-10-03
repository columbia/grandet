#include "decider.h"
#include "condition.h"
#include "log.h"

using namespace std;

Choice* Decider::choose(const Condition& condition, double* cost) {
    Choice* bestChoice = nullptr;
    double minCost = -1;
    for (auto& choice : choices) {
        if (choice.satisfy(condition)) {
            double cost = choice.cost(condition);
            if (bestChoice == nullptr || minCost > cost) {
                minCost = cost;
                bestChoice = &choice;
            }
        }
    }
    Dbg() << "best choice cost: " << minCost << endl;
    if (cost) {
        *cost = minCost;
    }
    return bestChoice;
}

void Decider::addChoice(const Choice& choice) {
    choices.push_back(choice);
}

void Decider::clear() {
    choices.clear();
}

Choice* Decider::findBetter(const Condition& condition, const Choice& current,
        bool extraGet, double* cost) {
    Choice* bestChoice = nullptr;
    double minCost = -1;
    for (auto& choice : choices) {
        double cost = -1;
        if (choice.getId() != current.getId()) {
            if (choice.satisfy(condition)) {
                cost = choice.migrateCost(condition, current, extraGet);
            }
        } else {
            cost = choice.cost(condition);
        }
        if (cost >= -0.5) {
            if (bestChoice == nullptr || minCost > cost) {
                minCost = cost;
                bestChoice = &choice;
            }
        }
    }
    Dbg() << "best choice cost: " << minCost << endl;
    if (cost) {
        *cost = minCost;
    }
    if (bestChoice != nullptr && bestChoice->getId() != current.getId()) {
        return bestChoice;
    }
    return nullptr;
}
