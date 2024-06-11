#pragma once

#include "datastore.h"
#include "redis_type.h"

class Controller {
public:
    RedisType::RedisValue handleCommand(const std::vector<RedisType::BulkString> &command);

private:
    RedisType::RedisValue handleEcho(const std::vector<RedisType::BulkString> &command);
    RedisType::RedisValue handlePing(const std::vector<RedisType::BulkString> &command);
    RedisType::RedisValue handleSet(const std::vector<RedisType::BulkString> &command);
    RedisType::RedisValue handleGet(const std::vector<RedisType::BulkString> &command);
    RedisType::RedisValue handleExists(const std::vector<RedisType::BulkString> &command);


    DataStore dataStore;
};
