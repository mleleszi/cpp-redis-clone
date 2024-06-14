#pragma once

#include "controller.h"
#include <atomic>
#include <string>


class TCPServer {
public:
    explicit TCPServer();
    [[noreturn]] void start(const std::string &address, int port);
    void handleRequest(int conn_fd);

private:
    int m_serverFD;
    Controller controller;
    
    static constexpr size_t RECV_SIZE = 2048;
};

;
