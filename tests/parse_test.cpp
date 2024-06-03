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

TEST(ParseTests, ParseArray) {
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

TEST(ParseTests, FindSeparatorSeparatorFound) {
    std::string input = "AB\r\nCD";
    std::vector<uint8_t> buffer = stringToByteVector(input);
    size_t pos = findSeparator(buffer);
    EXPECT_EQ(pos, 2);
}

TEST(ParseTests, FindSeparatorSeparatorNotFound) {
    std::string input = "ABCD";
    std::vector<uint8_t> buffer = stringToByteVector(input);
    size_t pos = findSeparator(buffer);
    EXPECT_EQ(pos, std::string::npos);
}

TEST(ParseTests, FindSeparatorMultipleSeparators) {
    std::string input = "A\r\nB\r\nC";
    std::vector<uint8_t> buffer = stringToByteVector(input);
    size_t pos = findSeparator(buffer);
    EXPECT_EQ(pos, 1);
}

TEST(ParseTests, FindSeparatorEmptyBuffer) {
    std::string input = "";
    std::vector<uint8_t> buffer = stringToByteVector(input);
    size_t pos = findSeparator(buffer);
    EXPECT_EQ(pos, std::string::npos);
}

TEST(ParseTests, FindSeparatorSeparatorAtEnd) {
    std::string input = "ABC\r\n";
    std::vector<uint8_t> buffer = stringToByteVector(input);
    size_t pos = findSeparator(buffer);
    EXPECT_EQ(pos, 3);
}

TEST(ParseTests, ExtractStringFromBytes) {
    std::vector<uint8_t> buffer = {'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd'};
    std::string result = extractStringFromBytes(buffer, 0, 5);
    EXPECT_EQ(result, "Hello");
}

TEST(ParseTests, EncodeSimpleString) {
    RedisType::SimpleString simpleString{"Hello"};
    std::vector<uint8_t> expected = {'+', 'H', 'e', 'l', 'l', 'o', '\r', '\n'};
    auto result = encode(simpleString);

    EXPECT_EQ(result, expected);
}

TEST(ParseTests, EncodeInteger) {
    RedisType::Integer integer{42};
    std::vector<uint8_t> expected = {':', '4', '2', '\r', '\n'};
    auto result = encode(integer);

    EXPECT_EQ(result, expected);
}

TEST(ParseTests, EncodeSimpleError) {
    RedisType::SimpleError simpleError{"Error message"};
    std::vector<uint8_t> expected = {'-', 'E', 'r', 'r', 'o', 'r', ' ', 'm', 'e', 's', 's', 'a', 'g', 'e', '\r', '\n'};
    auto result = encode(simpleError);

    EXPECT_EQ(result, expected);
}

TEST(ParseTests, EncodeBulkString) {
    auto str = stringToByteVector("Hello, World!");
    RedisType::BulkString bulkString{str};
    std::vector<uint8_t> expected = {'$', '1', '3', '\r', '\n', 'H', 'e', 'l', 'l',  'o',
                                     ',', ' ', 'W', 'o',  'r',  'l', 'd', '!', '\r', '\n'};
    auto result = encode(bulkString);

    EXPECT_EQ(result, expected);
}

TEST(ParseTests, EncodeArray) {
    RedisType::Array array{std::vector<RedisType::RedisValue>{
            RedisType::SimpleString{"simple1"},
            RedisType::BulkString{stringToByteVector("bulk1")},
            RedisType::Integer{123},
            RedisType::SimpleError{"error1"},
    }};

    std::vector<uint8_t> expected = {
            '*', '4',  '\r', '\n', '+', 's', 'i', 'm', 'p', 'l',  'e',  '1',  '\r', '\n',
            '$', '5',  '\r', '\n', 'b', 'u', 'l', 'k', '1', '\r', '\n', ':',  '1',  '2',
            '3', '\r', '\n', '-',  'e', 'r', 'r', 'o', 'r', '1',  '\r', '\n',
    };

    auto result = encode(array);

    EXPECT_EQ(result, expected);
}
