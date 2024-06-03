#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <variant>
#include <vector>


#include "overloaded.h"
#include "redis_type.h"

static const std::string CLRF = "\r\n";
static const size_t CLRF_SIZE = CLRF.size();

static size_t findSeparator(const std::vector<uint8_t> &buffer) {
    auto it = std::search(buffer.begin(), buffer.end(), CLRF.begin(), CLRF.end());
    return it == buffer.end() ? std::string::npos : std::distance(buffer.begin(), it);
}

static std::string extractStringFromBytes(const std::vector<uint8_t> &buffer, size_t start, size_t length) {
    return {buffer.begin() + static_cast<long>(start),
            buffer.begin() + static_cast<long>(start) + static_cast<long>(length)};
}

static std::vector<uint8_t> stringToByteVector(const std::string &str) { return {str.begin(), str.end()}; }

static std::optional<std::pair<RedisType::RedisValue, size_t>> parseMessage(const std::vector<uint8_t> &buffer) {
    size_t separator = findSeparator(buffer);

    if (separator == std::string::npos) return std::nullopt;

    std::string payload = extractStringFromBytes(buffer, 1, separator - 1);
    char prefix = static_cast<char>(buffer[0]);

    switch (prefix) {
        case '+': {
            RedisType::SimpleString result{payload};
            return std::make_pair(result, separator + CLRF_SIZE);
        }
        case '-': {
            RedisType::SimpleError result{payload};
            return std::make_pair(result, separator + CLRF_SIZE);
        }
        case ':': {
            RedisType::Integer result{std::stoll(payload)};
            return std::make_pair(result, separator + CLRF_SIZE);
        }
        case '$': {
            int length = std::stoi(payload);

            if (length == -1) { return std::make_pair(RedisType::BulkString{std::nullopt}, 5); }

            size_t endOfMessage = separator + 2 + length;
            std::vector<uint8_t> data(buffer.begin() + static_cast<long>(separator) + 2,
                                      buffer.begin() + static_cast<long>(endOfMessage));
            return std::make_pair(RedisType::BulkString{data}, endOfMessage + CLRF_SIZE);
        }
        case '*': {
            int length = std::stoi(payload);

            if (length == -1) { return std::make_pair(RedisType::Array{std::nullopt}, separator + CLRF_SIZE); }

            if (length == 0) {
                return std::make_pair(RedisType::Array{std::vector<RedisType::RedisValue>{}}, separator + CLRF_SIZE);
            }

            std::vector<RedisType::RedisValue> array;

            size_t currentPos = separator;

            for (int i = 0; i < length; ++i) {
                auto nextElem =
                        parseMessage({buffer.begin() + static_cast<long>(currentPos + CLRF_SIZE), buffer.end()});
                if (nextElem) {
                    array.push_back(nextElem->first);
                    currentPos += nextElem->second;
                } else {
                    return std::nullopt;
                }
            }

            return std::make_pair(RedisType::Array{array}, currentPos + CLRF_SIZE);
        }
        default:
            return {};
    }
}


static std::vector<uint8_t> encode(const RedisType::RedisValue &message) {
    std::vector<uint8_t> encoded;

    std::visit(overloaded{
                       [&encoded](const RedisType::SimpleString &simpleString) {
                           auto encodedString = "+" + simpleString.data + CLRF;
                           encoded = stringToByteVector(encodedString);
                       },
                       [&encoded](const RedisType::SimpleError &simpleError) {
                           auto encodedError = "-" + simpleError.data + CLRF;
                           encoded = stringToByteVector(encodedError);
                       },
                       [&encoded](const RedisType::Integer &integer) {
                           auto encodedError = ":" + std::to_string(integer.data) + CLRF;
                           encoded = stringToByteVector(encodedError);
                       },
                       [&encoded](const RedisType::BulkString &bulkString) {
                           if (!bulkString.data.has_value()) {
                               encoded = stringToByteVector("$-1" + CLRF);
                               return;
                           }

                           if ((*bulkString.data).empty()) {
                               encoded = stringToByteVector("$0" + CLRF + CLRF);
                           } else {
                               auto encodedString =
                                       "$" + std::to_string((*bulkString.data).size()) + CLRF +
                                       extractStringFromBytes(*bulkString.data, 0, (*bulkString.data).size()) + CLRF;
                               encoded = stringToByteVector(encodedString);
                           }
                       },
                       [&encoded](const RedisType::Array &array) {
                           if (!array.data.has_value()) {
                               encoded = stringToByteVector("*-1" + CLRF);
                               return;
                           }

                           if ((*array.data).empty()) {
                               encoded = stringToByteVector("$0" + CLRF + CLRF);
                           } else {
                               auto encodedString = "*" + std::to_string((array.data)->size()) + CLRF;

                               encoded = stringToByteVector(encodedString);
                               for (const auto &element: *array.data) {
                                   auto elementEncoded = encode(element);
                                   encoded.insert(encoded.end(), elementEncoded.begin(), elementEncoded.end());
                               }
                           }
                       },
               },
               message);

    return encoded;
}