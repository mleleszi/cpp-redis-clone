add_executable(cpp_redis main.cpp
        redis_type.h
        protocol.h
        controller.cpp
        controller.h
        tcp_server.cpp
        tcp_server.h
        overloaded.h
        datastore.h
        datastore.cpp)


target_link_libraries(cpp_redis PRIVATE spdlog::spdlog_header_only)