#include "value.h"
#include <fstream>
#include <exception>

using namespace std;

class ValueCannotPerformThisOperationException : public exception {};

class StringValue : public Value {
    string content;

public:
    StringValue() {}

    StringValue(const string& src) {
        content = src;
    }

    void loot(string& src) {
        content.swap(src);
    }

    void loot(string&& src) {
        content.swap(src);
    }

    string toString() override {
        return content;
    }

    string toFilename() override {
        throw ValueCannotPerformThisOperationException();
    }

    string toUrl() override {
        throw ValueCannotPerformThisOperationException();
    }

    bool hasFilename() override {
        return false;
    }

    bool hasUrl() override {
        return false;
    }

    size_t getSize() const override {
        return content.size();
    }
};

class FileValue : public Value {
    string filename;

public:
    FileValue(const string& filename) {
        this->filename = filename;
    }

    string toString() override {
        ifstream input(filename);

        return string((istreambuf_iterator<char>(input)), istreambuf_iterator<char>());
    }

    string toFilename() override {
        return filename;
    }

    string toUrl() override {
        throw ValueCannotPerformThisOperationException();
    }

    bool hasFilename() override {
        return true;
    }

    bool hasUrl() override {
        return false;
    }

    size_t getSize() const override {
        ifstream input(filename);
        input.seekg(0, input.end);
        return input.tellg();
    }
};

class UrlValue : public Value {
    string url;

public:
    UrlValue(const string& url) {
        this->url = url;
    }

    string toString() override {
        throw ValueCannotPerformThisOperationException();
    }

    string toFilename() override {
        throw ValueCannotPerformThisOperationException();
    }

    string toUrl() override {
        return url;
    }

    bool hasFilename() override {
        return false;
    }

    bool hasUrl() override {
        return true;
    }

    size_t getSize() const override {
        throw ValueCannotPerformThisOperationException();
    }
};

Value* CreateStringValue(const std::string& src) {
    return new StringValue(src);
}

Value* LootStringValue(string& src) {
    StringValue *sValue = new StringValue();
    sValue->loot(src);
    return sValue;
}

Value* LootStringValue(string&& src) {
    StringValue *sValue = new StringValue();
    sValue->loot(src);
    return sValue;
}

Value* CreateFileValue(const std::string& filename) {
    return new FileValue(filename);
}

Value* CreateUrlValue(const std::string& url) {
    return new UrlValue(url);
}
