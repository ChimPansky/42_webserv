add_library(Utils
    logger.cpp
    utils.cpp
)

target_include_directories(Utils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
