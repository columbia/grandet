#ifndef _GRANDET_EBSACTOR_H_
#define _GRANDET_EBSACTOR_H_

struct Actor;

class EBSActor : public Actor {
public:
    void put(const Key& key, const Value& value);
    Value get(const Key& key);
};

#endif // _GRANDET_EBSACTOR_H_
