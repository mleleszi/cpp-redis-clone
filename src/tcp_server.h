#pragma once

#include "controller.h"
#include <atomic>
#include <optional>
#include <poll.h>
#include <string>
#include <unordered_map>
#include <vector>

class TCPServer {
public:
    explicit TCPServer(const std::optional<std::string> &writeAheadLogFileName);
    [[noreturn]] void start(const std::string &address, int port);
    void handleRequest(int conn_fd);

private:
    int m_serverFD;
    Controller controller;

    static constexpr size_t RECV_SIZE = 2048;
    std::vector<pollfd> poll_fds;
    std::unordered_map<int, std::vector<uint8_t>> buffers;
};