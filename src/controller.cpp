#include <algorithm>
#include <iostream>

#include "controller.h"
#include "parse.h"
#include "redis_type.h"


RedisType::RedisValue Controller::handleCommand(const std::vector<RedisType::BulkString> &command) {
    if (command.empty()) { return RedisType::SimpleError("ERR empty command"); }

    auto commandType = extractStringFromBytes(*command[0].data, 0, (*command[0].data).size());

    std::transform(commandType.begin(), commandType.end(), commandType.begin(), ::toupper);

    if (commandType == "ECHO") handleEcho(command);

    return RedisType::SimpleError("ERR unsupported command");
}
RedisType::RedisValue Controller::handleEcho(const std::vector<RedisType::BulkString> &command) {
    if (command.size() != 2) { return RedisType::SimpleError("ERR wrong number of arguments for 'echo' command"); }

    return command[1];
}
