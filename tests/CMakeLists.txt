include(FetchContent)

FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.14.0
)

FetchContent_MakeAvailable(googletest)

add_library(GTest::GTest INTERFACE IMPORTED)
target_link_libraries(GTest::GTest INTERFACE gtest_main)

add_executable(redis_test protocol_test.cpp
        controller_test.cpp
        ${CMAKE_SOURCE_DIR}/src/controller.cpp     #TODO: refactor
)

target_link_libraries(redis_test
        PRIVATE
        GTest::GTest)

target_include_directories(redis_test PRIVATE ${CMAKE_SOURCE_DIR}/src)

