add_library(Utils
    logger.cpp
    time_utils.cpp
    str_utils.cpp
)

target_include_directories(Utils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
