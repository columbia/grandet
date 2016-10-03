#include "key.h"
using namespace std;

string Key::toString() const {
    return content;
}

bool Key::operator<(const Key& other) const {
    return content < other.content;
}

Key::Key(const string& src) {
    content = src;
}
