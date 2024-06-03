#include "parse.h"
#include "gtest/gtest.h"

TEST(ParseTests, ParseSimpleString) {
    std::string buffer_str = "+OK\r\n";
    auto buffer = stringToByteVector(buffer_str);
    auto result = parseMessage(buffer);
    
    ASSERT_TRUE(result.has_value());
    ASSERT_TRUE(std::holds_alternative<RedisType::SimpleString>(result->first));
    EXPECT_EQ(std::get<RedisType::SimpleString>(result->first).data, "OK");
    EXPECT_EQ(result->second, 5);
}

TEST(ParseTests, ParseSimpleStringExtraData) {
    std::string buffer_str = "+OK\r\nhello";
    auto buffer = stringToByteVector(buffer_str);
    auto result = parseMessage(buffer);

    ASSERT_TRUE(result.has_value());
    ASSERT_TRUE(std::holds_alternative<RedisType::SimpleString>(result->first));
    EXPECT_EQ(std::get<RedisType::SimpleString>(result->first).data, "OK");
    EXPECT_EQ(result->second, 5);
}

TEST(ParseTests, ParseSimpleError) {
    std::string buffer_str = "-Error\r\n";
    auto buffer = stringToByteVector(buffer_str);
    auto result = parseMessage(buffer);

    ASSERT_TRUE(result.has_value());
    ASSERT_TRUE(std::holds_alternative<RedisType::SimpleError>(result->first));
    EXPECT_EQ(std::get<RedisType::SimpleError>(result->first).data, "Error");
    EXPECT_EQ(result->second, 8);
}

TEST(ParseTests, ParseInteger) {
    std::string buffer_str = ":124\r\n";
    auto buffer = stringToByteVector(buffer_str);
    auto result = parseMessage(buffer);

    ASSERT_TRUE(result.has_value());
    ASSERT_TRUE(std::holds_alternative<RedisType::Integer>(result->first));
    EXPECT_EQ(std::get<RedisType::Integer>(result->first).data, 124);
    EXPECT_EQ(result->second, 6);
}

TEST(ParseTests, ParseNegativeInteger) {
    std::string buffer_str = ":-124\r\n";
    auto buffer = stringToByteVector(buffer_str);
    auto result = parseMessage(buffer);

    ASSERT_TRUE(result.has_value());
    ASSERT_TRUE(std::holds_alternative<RedisType::Integer>(result->first));
    EXPECT_EQ(std::get<RedisType::Integer>(result->first).data, -124);
    EXPECT_EQ(result->second, 7);
}

TEST(ParseTests, ParseBulkString) {
    std::string buffer_str = ":-124\r\n";
    auto buffer = stringToByteVector(buffer_str);
    auto result = parseMessage(buffer);

    ASSERT_TRUE(result.has_value());
    ASSERT_TRUE(std::holds_alternative<RedisType::Integer>(result->first));
    EXPECT_EQ(std::get<RedisType::Integer>(result->first).data, -124);
    EXPECT_EQ(result->second, 7);
}

TEST(ParseTests, ParseNullBulkStringTest) {
    std::string buffer_str = "$-1\r\n";
    auto buffer = stringToByteVector(buffer_str);
    auto result = parseMessage(buffer);

    ASSERT_TRUE(result.has_value());
    ASSERT_TRUE(std::holds_alternative<RedisType::BulkString>(result->first));

    auto bulkData = std::get<RedisType::BulkString>(result->first).data;
    ASSERT_FALSE(bulkData.has_value());
    EXPECT_EQ(result->second, 5);
}

TEST(ParseTests, ParseEmptyBulkStringTest) {
    std::string buffer_str = "$0\r\n\r\n";
    auto buffer = stringToByteVector(buffer_str);
    auto result = parseMessage(buffer);

    ASSERT_TRUE(result.has_value());
    ASSERT_TRUE(std::holds_alternative<RedisType::BulkString>(result->first));

    auto bulkData = std::get<RedisType::BulkString>(result->first).data;
    ASSERT_TRUE(bulkData.has_value());
    auto str = extractStringFromBytes(*bulkData, 0, bulkData->size());
    EXPECT_EQ(str, "");
    EXPECT_EQ(result->second, 6);
}

TEST(ParseTests, ParseBulkStringTest) {
    std::string buffer_str = "$11\r\nHello World\r\n";
    auto buffer = stringToByteVector(buffer_str);
    auto result = parseMessage(buffer);

    ASSERT_TRUE(result.has_value());
    ASSERT_TRUE(std::holds_alternative<RedisType::BulkString>(result->first));

    auto bulkData = std::get<RedisType::BulkString>(result->first).data;
    ASSERT_TRUE(bulkData.has_value());
    auto str = extractStringFromBytes(*bulkData, 0, bulkData->size());
    EXPECT_EQ(str, "Hello World");
    EXPECT_EQ(result->second, 18);
}

TEST(ParseTests, ArrayTest) {
    std::string buffer_str = "*2\r\n+OK\r\n:123\r\n";
    auto buffer = stringToByteVector(buffer_str);
    auto result = parseMessage(buffer);

    ASSERT_TRUE(result.has_value());
    ASSERT_TRUE(std::holds_alternative<RedisType::Array>(result->first));
    auto arrayData = std::get<RedisType::Array>(result->first).data;
    ASSERT_TRUE(arrayData.has_value());
    EXPECT_EQ(arrayData->size(), 2);

    EXPECT_TRUE(std::holds_alternative<RedisType::SimpleString>((*arrayData)[0]));
    EXPECT_TRUE(std::holds_alternative<RedisType::Integer>((*arrayData)[1]));

    EXPECT_EQ(std::get<RedisType::SimpleString>((*arrayData)[0]).data, "OK");
    EXPECT_EQ(std::get<RedisType::Integer>((*arrayData)[1]).data, 123);
    EXPECT_EQ(result->second, 15);
}
