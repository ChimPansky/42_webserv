add_library(Utils
    errors.cpp
    file_utils.cpp
    logger.cpp
    rand.cpp
    str_utils.cpp
    time_utils.cpp
)

target_include_directories(Utils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
