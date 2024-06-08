#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace RedisType {
    struct SimpleString {
        std::string data;

        explicit SimpleString(std::string data) : data(std::move(data)) {}
    };

    struct BulkString {
        std::optional<std::vector<uint8_t>> data;

        BulkString() = default;
        BulkString(const std::optional<std::vector<uint8_t>> &vec) : data(vec) {}
        BulkString(const std::string &str) : data(std::vector<uint8_t>(str.begin(), str.end())) {}
    };

    struct Integer {
        std::int64_t data;
    };

    struct SimpleError {
        explicit SimpleError(std::string errorMsg) : data(std::move(errorMsg)) {}

        std::string data;
    };

    struct Array {
        std::optional<std::vector<std::variant<SimpleString, BulkString, Integer, SimpleError, Array>>> data;
    };

    using RedisValue = std::variant<SimpleString, BulkString, Integer, SimpleError, Array>;
};// namespace RedisType