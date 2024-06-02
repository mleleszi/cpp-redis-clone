#include "parse.h"
#include "gtest/gtest.h"

TEST(ParseTests, SimpleStringTest) {
    std::string buffer_str = "+OK\r\n";
    auto buffer = stringToByteVector(buffer_str);
    auto result = parseMessage(buffer);
    
    ASSERT_TRUE(result.has_value());
    ASSERT_TRUE(std::holds_alternative<RedisType::SimpleString>(result->first));
    EXPECT_EQ(std::get<RedisType::SimpleString>(result->first).data, "OK");
    EXPECT_EQ(result->second, 5);
}

TEST(ParseTests, SimpleStringTestExtraData) {
    std::string buffer_str = "+OK\r\nhello";
    auto buffer = stringToByteVector(buffer_str);
    auto result = parseMessage(buffer);

    ASSERT_TRUE(result.has_value());
    ASSERT_TRUE(std::holds_alternative<RedisType::SimpleString>(result->first));
    EXPECT_EQ(std::get<RedisType::SimpleString>(result->first).data, "OK");
    EXPECT_EQ(result->second, 5);
}