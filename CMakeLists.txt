# MyProject/CMakeLists.txt
cmake_minimum_required(VERSION 3.14)
project(Webserv)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.15.2
)

FetchContent_MakeAvailable(googletest)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 98)
add_compile_options(-Wall -Wextra -Werror)
add_compile_options("$<$<CONFIG:Debug>:-gdwarf-2>")  # valgrid compatibility
add_compile_options("$<$<CONFIG:Debug>:-Og>")
# add_compile_options(-fsanitize=address,undefined,leak)
# add_link_options(-fsanitize=address,undefined,leak)

set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add subdirectories
add_subdirectory(src/c_api)
add_subdirectory(src/config)
add_subdirectory(src/http)
add_subdirectory(src/server)
add_subdirectory(src/utils)

# Main executable
add_executable(webserv
    src/main.cpp
    src/ClientSession.cpp
    src/ServerCluster.cpp
)
target_link_libraries(webserv PRIVATE
    CApi
    Utils
    Http
    Server
    Config
)

enable_testing()
add_subdirectory(tests)
