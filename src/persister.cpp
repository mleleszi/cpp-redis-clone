#include "persister.h"
#include "controller.h"

WriteAheadLogPersister::WriteAheadLogPersister(const std::string &fileName)
    : file(fileName, std::ios::binary | std::ios::app) {
    if (!file.is_open()) {
        spdlog::warn("Error opening file  {} for restoration", fileName);
        return;
    }
}
WriteAheadLogPersister::~WriteAheadLogPersister() { file.close(); }

void WriteAheadLogPersister::writeAndFlush(const std::vector<uint8_t> &data) {
    file.write(reinterpret_cast<const char *>(data.data()), static_cast<long>(data.size()));
    file.flush();
}
void WriteAheadLogPersister::restoreFromFile(const std::string &fileName, Controller &controller) {
    std::vector<uint8_t> buffer;
    std::ifstream file(fileName, std::ios::binary);

    if (!file) {
        spdlog::warn("Error opening file  {} for restoration", fileName);
        return;
    }

    const std::size_t RECV_SIZE = 2048;

    while (file) {
        std::vector<uint8_t> data(RECV_SIZE);

        // Read from file
        file.read(reinterpret_cast<char *>(data.data()), RECV_SIZE);
        std::streamsize bytes_received = file.gcount();

        if (bytes_received <= 0) { break; }

        buffer.insert(buffer.end(), data.begin(), data.begin() + bytes_received);

        // Parse message
        auto parsed = parseMessage(buffer);
        if (!parsed) {
            continue;// Wait for more data
        }

        auto [message, length] = *parsed;

        if (!std::holds_alternative<RedisType::Array>(message)) { break; }

        // If successfully parsed message, then erase
        buffer.erase(buffer.begin(), buffer.begin() + static_cast<long>(length));

        auto array = std::get<RedisType::Array>(message).data;

        if (!array) { break; }

        // Convert message to internal command format
        std::vector<RedisType::BulkString> command;
        bool err = false;
        for (const auto &item: *array) {
            if (!std::holds_alternative<RedisType::BulkString>(item)) {
                err = true;
                break;
            }
            command.push_back(std::get<RedisType::BulkString>(item));
        }

        if (err) { break; }

        // Handle command
        RedisType::RedisValue res = controller.handleCommand(command);
        auto encoded = encode(res);
        spdlog::debug("Restored: {}", std::get<RedisType::Array>(message), res);
    }

    file.close();
}
