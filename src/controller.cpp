#include <algorithm>
#include <iostream>
#include <numeric>


#include "controller.h"
#include "protocol.h"
#include "redis_type.h"

Controller::Controller(const std::optional<std::string> &writeAheadLogFileName) : persister{writeAheadLogFileName} {
    dataStore.startExpiryDaemon();
}

Controller::Controller() { dataStore.startExpiryDaemon(); }

RedisType::RedisValue Controller::handleCommand(const std::vector<RedisType::BulkString> &command) {
    if (command.empty()) { return RedisType::SimpleError("ERR empty command"); }

    auto commandType = extractStringFromBytes(*command[0].data, 0, (*command[0].data).size());

    std::transform(commandType.begin(), commandType.end(), commandType.begin(), ::toupper);

    if (commandType == "ECHO") {
        return handleEcho(command);
    } else if (commandType == "PING") {
        return handlePing(command);
    } else if (commandType == "SET") {
        return handleSet(command, true);
    } else if (commandType == "GET") {
        return handleGet(command);
    } else if (commandType == "EXISTS") {
        return handleExists(command);
    } else if (commandType == "CONFIG") {
        return handleConfig(command);
    }

    return RedisType::SimpleError("ERR unsupported command");
}

RedisType::RedisValue Controller::handleEcho(const std::vector<RedisType::BulkString> &command) {
    if (command.size() != 2) { return RedisType::SimpleError("ERR wrong number of arguments for 'echo' command"); }

    return command[1];
}
RedisType::RedisValue Controller::handlePing(const std::vector<RedisType::BulkString> &command) {
    if (command.size() > 2) { return RedisType::SimpleError("ERR wrong number of arguments for 'ping' command"); }
    if (command.size() == 2) { return command[1]; }

    return RedisType::SimpleString("PONG");
}

RedisType::RedisValue Controller::handleSet(const std::vector<RedisType::BulkString> &command, bool persist) {
    if (command.size() < 3 || command.size() > 5) {
        return RedisType::SimpleError("ERR wrong number of arguments for 'set' command");
    }

    auto key = extractStringFromBytes(*command[1].data, 0, (*command[1].data).size());
    auto val = extractStringFromBytes(*command[2].data, 0, (*command[2].data).size());

    int expireTimeMillis = -1;

    for (int i = 3; i < command.size(); ++i) {
        std::string option = extractStringFromBytes(*command[i].data, 0, (*command[i].data).size());
        std::transform(option.begin(), option.end(), option.begin(), ::toupper);

        if (option == "EX" && i + 1 < command.size()) {
            try {
                expireTimeMillis =
                        std::stoi(extractStringFromBytes(*command[i + 1].data, 0, (*command[i + 1].data).size())) *
                        1000;
                ++i;
            } catch (...) { return RedisType::SimpleError("ERR syntax error"); }
        } else if (option == "PX" && i + 1 < command.size()) {
            try {
                expireTimeMillis =
                        std::stoi(extractStringFromBytes(*command[i + 1].data, 0, (*command[i + 1].data).size()));
                ++i;
            } catch (...) { return RedisType::SimpleError("ERR syntax error"); }
        } else {
            return RedisType::SimpleError("ERR syntax error");
        }
    }


    if (persist && persister) { persister->writeAndFlush(fileEncode(command)); }

    if (expireTimeMillis > 0) {
        dataStore.setWithExpiry(key, val,
                                std::chrono::system_clock::now() + std::chrono::milliseconds(expireTimeMillis));
    } else {
        dataStore.set(key, val);
    }


    return RedisType::SimpleString("OK");
}

RedisType::RedisValue Controller::handleGet(const std::vector<RedisType::BulkString> &command) {
    if (command.size() != 2) { return RedisType::SimpleError("ERR wrong number of arguments for 'get' command"); }

    auto key = extractStringFromBytes(*command[1].data, 0, (*command[1].data).size());

    auto valueOpt = dataStore.get(key);

    if (valueOpt) { return RedisType::BulkString(*valueOpt); }

    return RedisType::BulkString();
}
RedisType::RedisValue Controller::handleExists(const std::vector<RedisType::BulkString> &command) {
    if (command.size() < 2) { return RedisType::SimpleError("ERR wrong number of arguments for 'exists' command"); }

    int count =
            std::accumulate(command.begin() + 1, command.end(), 0, [this](int acc, const RedisType::BulkString &cmd) {
                auto key = extractStringFromBytes(*cmd.data, 0, cmd.data->size());
                return acc + (dataStore.exists(key) ? 1 : 0);
            });

    return RedisType::Integer(count);
}

RedisType::RedisValue Controller::handleConfig(const std::vector<RedisType::BulkString> &command) {
    return RedisType::Array();
}
