add_library(CApi
    multiplexers/EpollMultiplexer.cpp
    multiplexers/IMultiplexer.cpp
    multiplexers/SelectMultiplexer.cpp
    ClientSocket.cpp
    EventManager.cpp
    MasterSocket.cpp
    c_api_utils.cpp
)

target_link_libraries(CApi PUBLIC Utils)

target_include_directories(CApi PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
