add_library(CApi
    multiplexers/EpollMultiplexer.cpp
    multiplexers/AMultiplexer.cpp
    multiplexers/PollMultiplexer.cpp
    multiplexers/SelectMultiplexer.cpp
    c_api_utils.cpp
    ClientSocket.cpp
    EventManager.cpp
    MasterSocket.cpp
    Socket.cpp
    ChildProcessesManager.cpp
)

target_link_libraries(CApi PUBLIC Utils)

target_include_directories(CApi PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
