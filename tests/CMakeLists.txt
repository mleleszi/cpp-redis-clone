add_library(GTest::GTest INTERFACE IMPORTED)
target_link_libraries(GTest::GTest INTERFACE gtest_main)

add_executable(redis_test protocol_test.cpp
        controller_test.cpp
        ${CMAKE_SOURCE_DIR}/src/controller.cpp #TODO: refactor
        ${CMAKE_SOURCE_DIR}/src/datastore.cpp
        ${CMAKE_SOURCE_DIR}/src/persister.cpp
        datastore_test.cpp
)

target_link_libraries(redis_test
        PRIVATE
        GTest::GTest
        spdlog::spdlog_header_only)

target_include_directories(redis_test PRIVATE ${CMAKE_SOURCE_DIR}/src)

