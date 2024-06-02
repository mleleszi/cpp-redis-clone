//
// Created by Marcell on 02/06/2024.
//

#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "redis_type.h"

static const std::string MSG_SEPARATOR = "\r\n";
static const size_t MSG_SEPARATOR_SIZE = MSG_SEPARATOR.size();

static size_t findSeparator(const std::vector<uint8_t> &buffer) {
    auto it = std::search(buffer.begin(), buffer.end(), MSG_SEPARATOR.begin(), MSG_SEPARATOR.end());
    return it == buffer.end() ? std::string::npos : std::distance(buffer.begin(), it);
}

static std::string extractStringFromBytes(const std::vector<uint8_t> &buffer, size_t start, size_t length) {
    return {buffer.begin() + static_cast<long>(start), buffer.begin() + static_cast<long>(start) + static_cast<long>(length)};
}

static std::optional<std::pair<RedisType::RedisValue, size_t>> extractFrameFromBuffer(const std::vector<uint8_t> &buffer) {
    size_t separator = findSeparator(buffer);

    if (separator == std::string::npos)
        return std::nullopt;

    std::string payload = extractStringFromBytes(buffer, 1, separator - 1);
    char prefix = static_cast<char>(buffer[0]);

    switch (prefix) {
        case '+': {
            RedisType::SimpleString result{payload};
            return std::make_pair(result, separator + MSG_SEPARATOR_SIZE);
        }
        case '-': {
            RedisType::SimpleError result{payload};
            return std::make_pair(result, separator + MSG_SEPARATOR_SIZE);
        }
        case ':': {
            RedisType::Integer result{std::stoll(payload)};
            return std::make_pair(result, separator + MSG_SEPARATOR_SIZE);
        }
        default:
            return {};
    }


    return {};
};