cmake_minimum_required(VERSION 3.11)

project(ConfigParserTests LANGUAGES CXX)

enable_testing()

include(GoogleTest)
include(CTest)


add_library(ConfigParser STATIC
    src/config/utils.h
    src/config/utils.cpp
    src/config/ConfigParser.cpp
    src/config/ConfigParser.h
    src/config/Config.cpp
    src/config/Config.h
    src/config/HttpConfig.cpp
    src/config/HttpConfig.h
    src/config/ServerConfig.cpp
    src/config/ServerConfig.h
    src/config/LocationConfig.cpp
    src/config/LocationConfig.h
    src/utils/logger.h
    src/utils/logger.cpp
    src/utils/utils.h
    src/c_api/utils.h
    src/c_api/utils.cpp
)

include_directories(src)
target_include_directories(ConfigParser PRIVATE src)

include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.12.0
)
FetchContent_MakeAvailable(googletest)

add_executable(config_tester test_configs/config_parser_test.cpp)
target_compile_definitions(config_tester PRIVATE CMAKE_CURRENT_SOURCE_DIR="${CMAKE_SOURCE_DIR}")

target_link_libraries(config_tester
    ConfigParser
    gtest
    gtest_main
)

gtest_discover_tests(config_tester
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)
