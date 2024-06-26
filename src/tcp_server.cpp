#include <algorithm>
#include <arpa/inet.h>
#include <cstdlib>
#include <string>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>
#include <vector>

#include "spdlog/spdlog.h"

#include "controller.h"
#include "protocol.h"
#include "redis_type.h"
#include "tcp_server.h"

TCPServer::TCPServer(const std::optional<std::string> &writeAheadLogFileName) : controller{writeAheadLogFileName} {
    m_serverFD = socket(AF_INET, SOCK_STREAM, 0);

    if (m_serverFD < 0) { throw std::runtime_error("Failed to create server socket!"); }

    int reuse = 1;
    if (setsockopt(m_serverFD, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse)) < 0) {
        throw std::runtime_error("Setsockopt failed!");
    }

    if (writeAheadLogFileName) {
        spdlog::info("Write-Ahead Log enabled.");
        WriteAheadLogPersister::restoreFromFile(*writeAheadLogFileName, controller);
    } else {
        spdlog::info("Write-Ahead Log disabled.");
    }
}

[[noreturn]] void TCPServer::start(const std::string &address = "0.0.0.0", int port = 6379) {
    struct sockaddr_in server_addr {};
    server_addr.sin_family = AF_INET;

    if (address == "0.0.0.0") {
        server_addr.sin_addr.s_addr = INADDR_ANY;
    } else {
        inet_pton(AF_INET, address.c_str(), &server_addr.sin_addr);
    }
    server_addr.sin_port = htons(port);

    if (bind(m_serverFD, (struct sockaddr *) &server_addr, sizeof(server_addr)) != 0) {
        throw std::runtime_error("Port " + std::to_string(port) + " already in use!");
    }

    int connection_backlog = 128;
    if (listen(m_serverFD, connection_backlog) != 0) { throw std::runtime_error("Listen failed!"); }

    spdlog::info("Listening on port {}", port);

    while (true) {
        struct sockaddr_in client_addr = {};
        socklen_t socklen = sizeof(client_addr);

        int connFD = accept(m_serverFD, (struct sockaddr *) &client_addr, &socklen);

        if (connFD < 0) {
            continue;// error
        }

        char clientIP[INET_ADDRSTRLEN];

        if (inet_ntop(AF_INET, &(client_addr.sin_addr), clientIP, INET_ADDRSTRLEN) != nullptr) {
            int clientPort = ntohs(client_addr.sin_port);
            spdlog::info("Client connected from {}:{}", clientIP, clientPort);
        }

        std::thread(&TCPServer::handleRequest, this, connFD).detach();
    }
}

void TCPServer::handleRequest(int connFD) {
    std::vector<uint8_t> buffer;

    while (true) {
        std::vector<uint8_t> data(RECV_SIZE);

        // Read from socket
        ssize_t bytes_received = recv(connFD, data.data(), RECV_SIZE, 0);

        if (bytes_received <= 0) {
            close(connFD);
            break;
        }

        buffer.insert(buffer.end(), data.begin(), data.begin() + bytes_received);

        // Parse message
        auto [message, length] = *parseMessage(buffer);

        if (!std::holds_alternative<RedisType::Array>(message)) {
            close(connFD);
            break;
        }

        // If successfully parsed message, then erase
        buffer.erase(buffer.begin(), buffer.begin() + static_cast<long>(length));

        auto array = std::get<RedisType::Array>(message).data;

        if (!array) {
            close(connFD);
            break;
        }

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

        if (err) {
            close(connFD);
            break;
        }

        // Handle command
        RedisType::RedisValue res = controller.handleCommand(command);
        auto encoded = encode(res);
        spdlog::debug("Request: {}, Response: {}", std::get<RedisType::Array>(message), res);

        // Send response
        send(connFD, encoded.data(), encoded.size(), 0);
    }
}
