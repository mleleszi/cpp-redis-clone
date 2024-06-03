#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace RedisType {
    struct SimpleString {
        std::string data;
    };

    struct BulkString {
        std::optional<std::vector<uint8_t>> data;
    };

    struct Integer {
        std::int64_t data;
    };

    struct SimpleError {
        std::string data;
    };

    struct Array {
        std::optional<std::vector<std::variant<SimpleString, BulkString, Integer, SimpleError, Array>>> data;
    };

    using RedisValue = std::variant<SimpleString, BulkString, Integer, SimpleError, Array>;
};// namespace RedisType