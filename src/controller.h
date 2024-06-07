#pragma once

#include "redis_type.h"

class Controller {
public:
    RedisType::RedisValue handleCommand(const std::vector<RedisType::BulkString> &command);

private:
    RedisType::RedisValue handleEcho(const std::vector<RedisType::BulkString> &command);
};