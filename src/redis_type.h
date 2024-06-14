#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "spdlog/fmt/ostr.h"

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
        Integer(const int64_t &data) : data(data) {}
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

static std::ostream &operator<<(std::ostream &os, const RedisType::SimpleString &value) {
    return os << "SimpleString: " << value.data;
}

static std::ostream &operator<<(std::ostream &os, const RedisType::BulkString &value) {
    os << "BulkString: ";
    if (value.data.has_value()) {
        for (auto byte: value.data.value()) { os << static_cast<char>(byte); }
    } else {
        os << "null";
    }
    return os;
}

static std::ostream &operator<<(std::ostream &os, const RedisType::Integer &value) {
    return os << "Integer: " << value.data;
}

static std::ostream &operator<<(std::ostream &os, const RedisType::SimpleError &value) {
    return os << "SimpleError: " << value.data;
}

static std::ostream &operator<<(std::ostream &os, const RedisType::Array &value) {
    os << "Array: ";
    if (value.data.has_value()) {
        os << "[";
        bool first = true;
        for (const auto &elem: value.data.value()) {
            if (!first) { os << ", "; }
            first = false;
            std::visit([&os](const auto &v) { os << v; }, elem);
        }
        os << "]";
    } else {
        os << "null";
    }
    return os;
}

static std::ostream &operator<<(std::ostream &os, const RedisType::RedisValue &value) {
    std::visit([&os](const auto &v) { os << v; }, value);
    return os;
}

template<>
struct fmt::formatter<RedisType::SimpleString> {
    constexpr auto parse(fmt::format_parse_context &ctx) { return ctx.begin(); }

    template<typename FormatContext>
    auto format(const RedisType::SimpleString &value, FormatContext &ctx) {
        return fmt::format_to(ctx.out(), "SimpleString: {}", value.data);
    }
};

template<>
struct fmt::formatter<RedisType::BulkString> {
    constexpr auto parse(fmt::format_parse_context &ctx) { return ctx.begin(); }

    template<typename FormatContext>
    auto format(const RedisType::BulkString &value, FormatContext &ctx) {
        if (value.data.has_value()) {
            std::string str(value.data->begin(), value.data->end());
            return fmt::format_to(ctx.out(), "BulkString: {}", str);
        } else {
            return fmt::format_to(ctx.out(), "BulkString: null");
        }
    }
};

template<>
struct fmt::formatter<RedisType::Integer> {
    constexpr auto parse(fmt::format_parse_context &ctx) { return ctx.begin(); }

    template<typename FormatContext>
    auto format(const RedisType::Integer &value, FormatContext &ctx) {
        return fmt::format_to(ctx.out(), "Integer: {}", value.data);
    }
};

template<>
struct fmt::formatter<RedisType::SimpleError> {
    constexpr auto parse(fmt::format_parse_context &ctx) { return ctx.begin(); }

    template<typename FormatContext>
    auto format(const RedisType::SimpleError &value, FormatContext &ctx) {
        return fmt::format_to(ctx.out(), "SimpleError: {}", value.data);
    }
};

template<>
struct fmt::formatter<RedisType::Array> {
    constexpr auto parse(fmt::format_parse_context &ctx) { return ctx.begin(); }

    template<typename FormatContext>
    auto format(const RedisType::Array &value, FormatContext &ctx) {
        if (value.data.has_value()) {
            std::string result = "Array: [";
            bool first = true;
            for (const auto &elem: value.data.value()) {
                if (!first) { result += ", "; }
                first = false;
                result += std::visit([](const auto &v) { return fmt::format("{}", v); }, elem);
            }
            result += "]";
            return fmt::format_to(ctx.out(), "{}", result);
        } else {
            return fmt::format_to(ctx.out(), "Array: null");
        }
    }
};

template<>
struct fmt::formatter<RedisType::RedisValue> {
    constexpr auto parse(fmt::format_parse_context &ctx) { return ctx.begin(); }

    template<typename FormatContext>
    auto format(const RedisType::RedisValue &value, FormatContext &ctx) {
        return std::visit([&ctx](const auto &v) { return fmt::format_to(ctx.out(), "{}", v); }, value);
    }
};