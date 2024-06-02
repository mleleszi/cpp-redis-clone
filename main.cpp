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

    std::string buffer_str3 = "$5\r\nHello\r\n";
    auto buffer3 = stringToByteVector(buffer_str3);
    auto result3 = extractFrameFromBuffer(buffer3);

    std::cout << std::holds_alternative<RedisType::BulkString>(result3->first) << std::endl;

    auto res3 = std::get<RedisType::BulkString>(result3->first).data;
    auto str = std::string{std::begin(*res3), std::end(*res3)};
    std::cout << str << std::endl;

    std::string buffer_str4 = "*2\r\n+OK\r\n:123\r\n";
    auto buffer4 = stringToByteVector(buffer_str4);
    auto result4 = extractFrameFromBuffer(buffer4);


    std::cout << "array " << std::holds_alternative<RedisType::Array>(result4->first) << std::endl;
    auto res4 = *std::get<RedisType::Array>(result4->first).data;
    auto firstElem = std::get<RedisType::SimpleString>(res4[0]).data;
    auto secondElem = std::get<RedisType::Integer>(res4[1]).data;
    std::cout << firstElem << " " << secondElem << std::endl;
    
    return 0;
}
