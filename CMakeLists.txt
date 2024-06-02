cmake_minimum_required(VERSION 3.27)
project(cpp_redis)

set(CMAKE_CXX_STANDARD 23)

enable_testing()

add_subdirectory(src)
add_subdirectory(tests)

