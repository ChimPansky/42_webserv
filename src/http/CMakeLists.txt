add_library(Http
    Request.cpp
    RequestBuilder.cpp
    RequestParser.cpp
    Response.cpp
)

target_link_libraries(Http PUBLIC Utils)
target_link_libraries(Http PUBLIC CApi)

target_include_directories(Http PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
