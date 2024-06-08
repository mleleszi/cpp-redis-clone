#include "controller.h"
#include "parse.h"
#include "redis_type.h"
#include "gtest/gtest.h"


TEST(ControllerTests, HandleECHOInvalidNumArgs) {
    Controller controller;
    std::vector<RedisType::BulkString> command{RedisType::BulkString("ECHO")};
    auto result = controller.handleCommand(command);

    ASSERT_TRUE(std::holds_alternative<RedisType::SimpleError>(result));
    auto errorMsg = std::get<RedisType::SimpleError>(result).data;
    ASSERT_EQ(errorMsg, "ERR wrong number of arguments for 'echo' command");
}

TEST(ControllerTests, HandleECHO) {
    Controller controller;
    std::vector<RedisType::BulkString> command{RedisType::BulkString("ECHO"), RedisType::BulkString("Hello")};
    auto result = controller.handleCommand(command);

    ASSERT_TRUE(std::holds_alternative<RedisType::BulkString>(result));
    auto bulkData = *std::get<RedisType::BulkString>(result).data;
    ASSERT_EQ(bulkData, stringToByteVector("Hello"));
}

TEST(ControllerTests, HandleECHOIgnoreCase) {
    Controller controller;
    std::vector<RedisType::BulkString> command{RedisType::BulkString("EcHo"), RedisType::BulkString("Hello")};
    auto result = controller.handleCommand(command);

    ASSERT_TRUE(std::holds_alternative<RedisType::BulkString>(result));
    auto bulkData = *std::get<RedisType::BulkString>(result).data;
    ASSERT_EQ(bulkData, stringToByteVector("Hello"));
}

TEST(ControllerTests, HandlePINGInvalidNumArgs) {
    Controller controller;
    std::vector<RedisType::BulkString> command{RedisType::BulkString("PING"), RedisType::BulkString("Hello"),
                                               RedisType::BulkString("Hello")};
    auto result = controller.handleCommand(command);

    ASSERT_TRUE(std::holds_alternative<RedisType::SimpleError>(result));
    auto errorMsg = std::get<RedisType::SimpleError>(result).data;
    ASSERT_EQ(errorMsg, "ERR wrong number of arguments for 'ping' command");
}

TEST(ControllerTests, HandlePING) {
    Controller controller;
    std::vector<RedisType::BulkString> command{RedisType::BulkString("PING"), RedisType::BulkString("Hello")};
    auto result = controller.handleCommand(command);

    ASSERT_TRUE(std::holds_alternative<RedisType::BulkString>(result));
    auto bulkData = *std::get<RedisType::BulkString>(result).data;
    ASSERT_EQ(bulkData, stringToByteVector("Hello"));
}

TEST(ControllerTests, HandlePINGNoArg) {
    Controller controller;
    std::vector<RedisType::BulkString> command{RedisType::BulkString("PING")};
    auto result = controller.handleCommand(command);

    ASSERT_TRUE(std::holds_alternative<RedisType::SimpleString>(result));
    auto str = std::get<RedisType::SimpleString>(result).data;
    ASSERT_EQ(str, "PONG");
}
