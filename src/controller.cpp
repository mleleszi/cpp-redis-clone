#include <algorithm>
#include <iostream>

#include "controller.h"
#include "parse.h"
#include "redis_type.h"


std::ostream &operator<<(std::ostream &os, const RedisType::BulkString &bulkString) {
    if (bulkString.data) {
        for (auto byte: bulkString.data.value()) { os << static_cast<char>(byte); }
    } else {
        os << "nil";
    }
    return os;
}


RedisType::RedisValue Controller::handleCommand(const std::vector<RedisType::BulkString> &command) {
    for (const auto &item: command) { std::cout << item << std::endl; }

    if (command.empty()) { return RedisType::SimpleError("ERR empty command"); }

    auto commandType = extractStringFromBytes(*command[0].data, 0, (*command[0].data).size());

    std::transform(commandType.begin(), commandType.end(), commandType.begin(), ::toupper);
    
    if (commandType == "ECHO") { return handleEcho(command); }

    return RedisType::SimpleError("ERR unsupported command");
}

RedisType::RedisValue Controller::handleEcho(const std::vector<RedisType::BulkString> &command) {
    if (command.size() != 2) { return RedisType::SimpleError("ERR wrong number of arguments for 'echo' command"); }

    return command[1];
}
