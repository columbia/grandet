#include "nsestimator.h"
#include "key.h"
#include "log.h"

using namespace std;

string NamespaceEstimator::getClue(const Key& key, const Requirements& requirements) {
    string sKey = key.toString();
    int pos = sKey.find(":");
    if (pos != string::npos) {
        return sKey.substr(0, pos);
    } else {
        return sKey;
    }
}
