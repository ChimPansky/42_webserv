set(GENERATED_OUTPUT_DIR "${CMAKE_BINARY_DIR}/generated")

file(MAKE_DIRECTORY ${GENERATED_OUTPUT_DIR})

set(RESPONSE_CODES_H "${GENERATED_OUTPUT_DIR}/ResponseCodes.h")
set(RESPONSE_CODES_CPP "${GENERATED_OUTPUT_DIR}/ResponseCodes.cpp")

add_custom_command(
    OUTPUT ${RESPONSE_CODES_CPP} ${RESPONSE_CODES_H}
    COMMAND python3 ${PROJECT_SOURCE_DIR}/resources/response_code_gen.py ${CMAKE_CURRENT_SOURCE_DIR}/http_response_codes.src  ${RESPONSE_CODES_H} ${RESPONSE_CODES_CPP}
    DEPENDS ${PROJECT_SOURCE_DIR}/resources/response_code_gen.py
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/http_response_codes.src
    COMMENT "Generating ResponseCodes files"
)

add_library(Http
    Request.cpp
    RequestBuilder.cpp
    RequestParser.cpp
    Response.cpp
    ${RESPONSE_CODES_CPP}  # RESPONSE_CODES_CPP is the output of custom command, which creates implicit dependency on that command
)

target_link_libraries(Http PUBLIC Utils)
target_link_libraries(Http PUBLIC CApi)

target_include_directories(Http PUBLIC ${GENERATED_OUTPUT_DIR})
target_include_directories(Http PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
