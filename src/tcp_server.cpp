#include <algorithm>
#include <arpa/inet.h>
#include <cstdlib>
#include <iostream>
#include <ostream>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

#include "spdlog/spdlog.h"

#include "controller.h"
#include "parse.h"
#include "tcp_server.h"

TCPServer::TCPServer(const Controller &controller) : controller{controller} {
    m_serverFD = socket(AF_INET, SOCK_STREAM, 0);

    if (m_serverFD < 0) { throw std::runtime_error("Failed to create server socket!"); }

    int reuse = 1;
    if (setsockopt(m_serverFD, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse)) < 0) {
        throw std::runtime_error("Setsockopt failed!");
    }
}

[[noreturn]] void TCPServer::start(const std::string &address = "0.0.0.0", int port = 6379) {
    struct sockaddr_in server_addr {};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;// TODO: set address
    server_addr.sin_port = htons(port);

    if (bind(m_serverFD, (struct sockaddr *) &server_addr, sizeof(server_addr)) != 0) {
        throw std::runtime_error("Port " + std::to_string(port) + " already in use!");
    }

    int connection_backlog = 5;
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

        handleRequest(connFD);

        close(connFD);
    }
}

void TCPServer::handleRequest(int connFD) {
    std::vector<uint8_t> buffer;

    while (true) {
        std::vector<uint8_t> data(RECV_SIZE);

        // Read from socket
        ssize_t bytes_received = recv(connFD, data.data(), RECV_SIZE, 0);

        if (bytes_received <= 0) { break; }

        buffer.insert(buffer.end(), data.begin(), data.begin() + bytes_received);

        // Parse message
        auto [message, length] = *parseMessage(buffer);

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

        // Send response
        send(connFD, encoded.data(), encoded.size(), 0);
    }
}
