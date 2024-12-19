add_library(CApi
    multiplexers/EpollMultiplexer.cpp
    multiplexers/IMultiplexer.cpp
    multiplexers/PollMultiplexer.cpp
    multiplexers/SelectMultiplexer.cpp
    c_api_utils.cpp
    ClientSocket.cpp
    EventManager.cpp
    MasterSocket.cpp
    ReadSocket.cpp
)

target_link_libraries(CApi PUBLIC Utils)

target_include_directories(CApi PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
