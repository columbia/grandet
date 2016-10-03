#include "s3actor.h"
#include "gtest/gtest.h"

#include <string>
#include "config.h"
#include "key.h"
#include "value.h"
#include "profile.h"
#include "range.h"

using namespace std;

// fake
template<> int Config::get<int>(const string& path, const int& defaultValue) {
    return defaultValue;
}

// fake
bool Profile::get(const string& key, string* value) const {
    *value = "cloud-econ";
    return false;
}

namespace {

class S3ActorTest : public ::testing::Test {
protected:
    S3ActorTest() : key("test_key"), value_string("test_value") {
    }

    virtual ~S3ActorTest() override {
        delete s3Actor;
    }

    virtual void SetUp() override {
        ASSERT_NO_THROW(s3Actor = new S3Actor());
        Profile profile;
        s3Actor->del(key, profile);
    }

    virtual void TearDown() override {
        Profile profile;
        s3Actor->del(key, profile);
    }

    S3Actor *s3Actor;
    Key key;
    const string value_string;
};

TEST_F(S3ActorTest, PutAndGetTheSameObject) {
    shared_ptr<Value> value(CreateStringValue(value_string));
    Range range;
    EXPECT_NO_THROW({
        Profile profile;
        s3Actor->put(key, value, profile);
        value = s3Actor->get(key, profile);
    });
    EXPECT_EQ(value_string, value->toString());
}

TEST_F(S3ActorTest, PutDeleteAndGetTheSameObject) {
    shared_ptr<Value> value(CreateStringValue(value_string));
    Range range;
    EXPECT_NO_THROW({
        Profile profile;
        s3Actor->put(key, value, profile);
        s3Actor->del(key, profile);
        value = s3Actor->get(key, profile);
    });
    EXPECT_EQ(nullptr, value);
}

TEST_F(S3ActorTest, GetRange) {
    shared_ptr<Value> value(CreateStringValue(value_string));
    Range range{3, 5};
    EXPECT_NO_THROW({
        Profile profile;
        s3Actor->put(key, value, profile);
        value = s3Actor->get(key, profile, range);
    });
    EXPECT_EQ(value_string.substr(*range.start, *range.end - *range.start + 1),
            value->toString());
}

}  // namespace

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
