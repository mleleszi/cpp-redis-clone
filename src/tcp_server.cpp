#include <arpa/inet.h>
#include <cstdlib>
#include <fcntl.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <unordered_map>
#include <vector>

#include "controller.h"
#include "protocol.h"
#include "redis_type.h"
#include "spdlog/spdlog.h"
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

    // Set server socket to non-blocking mode
    fcntl(m_serverFD, F_SETFL, O_NONBLOCK);

    pollfd server_pollfd = {m_serverFD, POLLIN, 0};
    poll_fds.push_back(server_pollfd);

    spdlog::info("Listening on port {}", port);

    while (true) {
        int ret = poll(poll_fds.data(), poll_fds.size(), -1);

        if (ret < 0) {
            spdlog::error("Poll error: {}", strerror(errno));
            continue;
        }

        for (size_t i = 0; i < poll_fds.size(); ++i) {
            if (poll_fds[i].revents & POLLIN) {
                if (poll_fds[i].fd == m_serverFD) {
                    // Accept new connection
                    struct sockaddr_in client_addr = {};
                    socklen_t socklen = sizeof(client_addr);
                    int connFD = accept(m_serverFD, (struct sockaddr *) &client_addr, &socklen);

                    if (connFD >= 0) {
                        // Set client socket to non-blocking mode
                        fcntl(connFD, F_SETFL, O_NONBLOCK);

                        pollfd client_pollfd = {connFD, POLLIN, 0};
                        poll_fds.push_back(client_pollfd);
                        buffers[connFD] = std::vector<uint8_t>();

                        char clientIP[INET_ADDRSTRLEN];
                        if (inet_ntop(AF_INET, &(client_addr.sin_addr), clientIP, INET_ADDRSTRLEN) != nullptr) {
                            int clientPort = ntohs(client_addr.sin_port);
                            spdlog::info("Client connected from {}:{}", clientIP, clientPort);
                        }
                    }
                } else {
                    // Handle existing connection
                    handleRequest(poll_fds[i].fd);

                    // If the connection was closed, remove it from poll_fds and buffers
                    if (poll_fds[i].fd == -1) {
                        close(poll_fds[i].fd);
                        poll_fds.erase(poll_fds.begin() + i);
                        buffers.erase(poll_fds[i].fd);
                        --i;// Adjust index after removal
                    }
                }
            }
        }
    }
}

void TCPServer::handleRequest(int connFD) {
    std::vector<uint8_t> &buffer = buffers[connFD];

    std::vector<uint8_t> data(RECV_SIZE);

    // Read from socket
    ssize_t bytes_received = recv(connFD, data.data(), RECV_SIZE, 0);

    if (bytes_received < 0) {
        if (errno == EWOULDBLOCK || errno == EAGAIN) {
            // No data available for now, try again later
            return;
        } else {
            // Other error, close connection
            buffers.erase(connFD);
            close(connFD);
            connFD = -1;// Mark the connection as closed
            return;
        }
    } else if (bytes_received == 0) {
        // Connection closed by client
        buffers.erase(connFD);
        close(connFD);
        connFD = -1;// Mark the connection as closed
        return;
    }

    buffer.insert(buffer.end(), data.begin(), data.begin() + bytes_received);

    // Parse message
    auto [message, length] = *parseMessage(buffer);

    if (!std::holds_alternative<RedisType::Array>(message)) {
        buffers.erase(connFD);
        close(connFD);
        connFD = -1;// Mark the connection as closed
        return;
    }

    // If successfully parsed message, then erase
    buffer.erase(buffer.begin(), buffer.begin() + static_cast<long>(length));

    auto array = std::get<RedisType::Array>(message).data;

    if (!array) {
        buffers.erase(connFD);
        close(connFD);
        connFD = -1;// Mark the connection as closed
        return;
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
        buffers.erase(connFD);
        close(connFD);
        connFD = -1;// Mark the connection as closed
        return;
    }

    // Handle command
    RedisType::RedisValue res = controller.handleCommand(command);
    auto encoded = encode(res);
    spdlog::debug("Request: {}, Response: {}", std::get<RedisType::Array>(message), res);

    // Send response
    ssize_t bytes_sent = send(connFD, encoded.data(), encoded.size(), 0);
    if (bytes_sent < 0 && (errno != EWOULDBLOCK && errno != EAGAIN)) {
        // Error sending response
        buffers.erase(connFD);
        close(connFD);
        connFD = -1;// Mark the connection as closed
    }
}