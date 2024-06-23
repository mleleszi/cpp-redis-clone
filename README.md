# cpp-redis-clone

## Overview

In this repo you can find a multithreaded Redis clone written in C++. It implements the Redis serialization protocol, allowing it to be used seamlessly with the Redis CLI. Still work in progress.

## Features

- Supports the following RESP data types:
    - Integer 
    - SimpleString
    - BulkString
    - SimpleError
    - Array
- Implemented Commands: SET, GET, ECHO, PING, EXISTS

## Building

Only a C++ compiler and CMake need to be installed to build this application. The dependencies are fetched via CMake's
FetchContent.

Build and run:

```
git clone https://github.com/mleleszi/cpp-redis-clone
cd cpp-redis-clone
mkdir build
cd build
cmake .. && make
./src/cpp_redis
```

Dependencies:

- [`googletest`](https://github.com/google/googletest)
- [`spdlog`](https://github.com/gabime/spdlog)

  
  





