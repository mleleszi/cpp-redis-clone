#include <iostream>

#include "tcp_server.h"

int main() {
    TCPServer server;
    server.start("0.0.0.0", 6379);
}
