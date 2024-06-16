#include "spdlog/spdlog.h"
#include "tcp_server.h"

int main(int argc, char **argv) {
#if SPDLOG_ACTIVE_LEVEL == SPDLOG_LEVEL_DEBUG
    spdlog::set_level(spdlog::level::debug);
#elif SPDLOG_ACTIVE_LEVEL == SPDLOG_LEVEL_INFO
    spdlog::set_level(spdlog::level::info);
#endif

    std::optional<std::string> fileName;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--persist" || arg == "-p") {
            if (i + 1 < argc) {
                fileName = argv[i + 1];
            } else {
                spdlog::error("No filename provided after {}.", arg);
                return 1;
            }
            break;
        } else {
            spdlog::error("Unsupported argument: {}.", arg);
            return 1;
        }
    }

    TCPServer server(fileName);
    server.start("0.0.0.0", 6379);
}
