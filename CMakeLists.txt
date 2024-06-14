cmake_minimum_required(VERSION 3.27)
project(cpp_redis)

set(CMAKE_CXX_STANDARD 20)

include(FetchContent)

# TODO: create separate CMAKE for dependencies
FetchContent_Declare(
        spdlog
        GIT_REPOSITORY https://github.com/gabime/spdlog.git
        GIT_TAG v1.14.1
)
FetchContent_MakeAvailable(spdlog)

add_subdirectory(src)
add_subdirectory(tests)

