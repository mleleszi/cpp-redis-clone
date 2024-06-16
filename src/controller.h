#pragma once

#include "datastore.h"
#include "persister.h"
#include "redis_type.h"
#include <optional>

class Controller {
public:
    Controller();
    explicit Controller(const std::optional<std::string> &writeAheadLogFileName);

    RedisType::RedisValue handleCommand(const std::vector<RedisType::BulkString> &command);
    RedisType::RedisValue handleSet(const std::vector<RedisType::BulkString> &command, bool persist = false);

private:
    RedisType::RedisValue handleEcho(const std::vector<RedisType::BulkString> &command);
    RedisType::RedisValue handlePing(const std::vector<RedisType::BulkString> &command);
    RedisType::RedisValue handleGet(const std::vector<RedisType::BulkString> &command);
    RedisType::RedisValue handleExists(const std::vector<RedisType::BulkString> &command);
    RedisType::RedisValue handleConfig(const std::vector<RedisType::BulkString> &command);

    DataStore dataStore;
    std::optional<WriteAheadLogPersister> persister;
};
