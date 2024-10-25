add_library(Config
    Config.cpp
    ConfigBuilder.cpp
    HttpConfig.cpp
    HttpConfigBuilder.cpp
    InheritedSettings.cpp
    LocationConfig.cpp
    LocationConfigBuilder.cpp
    ParsedConfig.cpp
    ServerConfig.cpp
    ServerConfigBuilder.cpp
)

target_link_libraries(Config PUBLIC Utils)
target_link_libraries(Config PUBLIC CApi)

target_include_directories(Config PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
