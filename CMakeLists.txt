cmake_minimum_required(VERSION 3.27)
project(cpp_redis)

set(CMAKE_CXX_STANDARD 20)

# TODO: fetch spdlog with FetchContent
find_package(spdlog REQUIRED)

enable_testing()

add_subdirectory(src)
add_subdirectory(tests)

