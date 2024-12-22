add_compile_options(-Wno-deprecated)

# c_api
add_executable(c_api_test c_api_test.cpp)
target_link_libraries(c_api_test PRIVATE CApi gtest gtest_main)
add_test(
    NAME c_api_test
    COMMAND ./c_api_test
)

# RequestBuilder
add_executable(rq_builder_test rq_builder_test.cpp)
target_link_libraries(rq_builder_test PRIVATE Http gtest gtest_main)
add_test(
    NAME rq_builder_test
    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/rq_builder_test
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

# ConfigParser
add_executable(config_parser_test config_parser_test.cpp)
target_link_libraries(config_parser_test PRIVATE Config gtest gtest_main)
add_test(
    NAME config_parser_test
    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/config_parser_test
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/test_data/configs
)

# ServerMatching
add_executable(server_matching server_matching_test.cpp)
target_link_libraries(server_matching PRIVATE Server gtest gtest_main)
add_test(
    NAME server_matching
    COMMAND ./server_matching
)

# LocationMatching
add_executable(location_matching location_matching_test.cpp)
target_link_libraries(location_matching PRIVATE Server gtest gtest_main)
add_test(
    NAME location_matching
    COMMAND ./location_matching
)

# RequestTarget
add_executable(rq_target_test rq_target_test.cpp)
target_link_libraries(rq_target_test PRIVATE Http gtest gtest_main)
add_test(
    NAME rq_target_test
    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/rq_target_test
)

# SyntaxChecker
add_executable(syntax_checker_test syntax_checker_test.cpp)
target_link_libraries(syntax_checker_test PRIVATE Http gtest gtest_main)
add_test(
    NAME syntax_checker_test
    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/syntax_checker_test
)

# Valgrind
add_test(
    NAME valgrind_idle_test
    COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/valgrind_1_idle_test.sh
              "${CMAKE_BINARY_DIR}/webserv ${CMAKE_SOURCE_DIR}/test_data/valgrind_test.conf"
              ${CMAKE_BINARY_DIR}/Testing/simple_valgrind_test.out
)

add_test(
    NAME valgrind_single_connection_test
    COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/valgrind_2_single_connection_test.sh
              "${CMAKE_BINARY_DIR}/webserv ${CMAKE_SOURCE_DIR}/test_data/valgrind_test.conf"
              ${CMAKE_BINARY_DIR}/Testing/simple_valgrind_test.out
)
