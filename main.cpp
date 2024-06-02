#include <iostream>

#include "parse.h"
#include "redis_type.h"

std::vector<uint8_t> stringToByteVector(const std::string &str) {
    return {str.begin(), str.end()};
}

int main() {
    std::string buffer_str = "+OK\r\n";
    auto buffer = stringToByteVector(buffer_str);
    auto result = extractFrameFromBuffer(buffer);

    std::cout << std::holds_alternative<RedisType::SimpleString>(result->first) << std::endl;
    std::cout << std::get<RedisType::SimpleString>(result->first).data << std::endl;

    std::string buffer_str2 = ":-123\r\n";
    auto buffer2 = stringToByteVector(buffer_str2);
    auto result2 = extractFrameFromBuffer(buffer2);

    std::cout << std::holds_alternative<RedisType::Integer>(result2->first) << std::endl;
    std::cout << std::get<RedisType::Integer>(result2->first).data << std::endl;
    return 0;
}