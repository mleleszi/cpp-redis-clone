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


if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(cpp_redis PRIVATE SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_DEBUG)
elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_definitions(cpp_redis PRIVATE SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_INFO)
endif ()

target_link_libraries(cpp_redis PRIVATE spdlog::spdlog_header_only)