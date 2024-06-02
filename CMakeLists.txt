cmake_minimum_required(VERSION 3.27)
project(cpp_redis)

set(CMAKE_CXX_STANDARD 23)

add_executable(cpp_redis main.cpp
        redis_type.h
        parse.h)
