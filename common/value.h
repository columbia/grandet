#ifndef _GRANDET_VALUE_H_
#define _GRANDET_VALUE_H_

#include <string>

struct Value {
    virtual std::string toString() = 0;
    virtual std::string toFilename() = 0;
    virtual std::string toUrl() = 0;
    virtual bool hasFilename() = 0;
    virtual bool hasUrl() = 0;
    virtual size_t getSize() const = 0;
    virtual ~Value() = default;
};

Value* CreateStringValue(const std::string& src);
Value* LootStringValue(std::string& victim);
Value* LootStringValue(std::string&& victim);
Value* CreateFileValue(const std::string& filename);
Value* CreateUrlValue(const std::string& url);

#endif // _GRANDET_VALUE_H_
