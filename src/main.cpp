#include "spdlog/spdlog.h"
#include "tcp_server.h"

int main() {
#if SPDLOG_ACTIVE_LEVEL == SPDLOG_LEVEL_DEBUG
    spdlog::set_level(spdlog::level::debug);
#elif SPDLOG_ACTIVE_LEVEL == SPDLOG_LEVEL_INFO
    spdlog::set_level(spdlog::level::info);
#endif

    TCPServer server;
    server.start("0.0.0.0", 6379);
}
