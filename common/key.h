#ifndef _GRANDET_KEY_H_
#define _GRANDET_KEY_H_

#include <string>

class Key {
    std::string content;
public:
    std::string toString() const;
    bool operator<(const Key& other) const;

    Key(const std::string& src);
};

#endif // _GRANDET_KEY_H_
