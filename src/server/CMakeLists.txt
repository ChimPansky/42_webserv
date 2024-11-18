add_library(Server
    Server.cpp
    Location.cpp
)

target_link_libraries(Server PUBLIC Utils)
target_link_libraries(Server PUBLIC CApi)
target_link_libraries(Server PUBLIC Http)
target_link_libraries(Server PUBLIC Config)

target_include_directories(Server PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
