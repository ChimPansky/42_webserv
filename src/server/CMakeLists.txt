add_library(Server
    Location.cpp
    Server.cpp
    response_processors/AResponseProcessor.cpp
    response_processors/CGIProcessor.cpp
    response_processors/DirectoryProcessor.cpp
    response_processors/ErrorProcessor.cpp
    response_processors/FileProcessor.cpp
    response_processors/RedirectProcessor.cpp
    utils/utils.cpp
)

target_link_libraries(Server PUBLIC Utils)
target_link_libraries(Server PUBLIC CApi)
target_link_libraries(Server PUBLIC Http)
target_link_libraries(Server PUBLIC Config)

target_include_directories(Server PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
