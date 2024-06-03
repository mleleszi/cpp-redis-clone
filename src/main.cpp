#include <iostream>

#include "tcp_server.h"

int main() {
    Controller controller;
    TCPServer server{controller};
    server.start("0.0.0.0", 6379);
}
