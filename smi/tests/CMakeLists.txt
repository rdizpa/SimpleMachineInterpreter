include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/releases/download/v1.17.0/googletest-1.17.0.tar.gz
    URL_HASH SHA256=65fab701d9829d38cb77c14acdc431d2108bfdbf8979e40eb8ae567edf10b27c
)

set(INSTALL_GTEST OFF)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/files DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_executable(lexer_test lexer_test.cpp)
target_link_libraries(lexer_test GTest::gtest_main smi_lib)

add_executable(parser_test parser_test.cpp)
target_link_libraries(parser_test GTest::gtest_main smi_lib)

add_executable(smi_interpreter_test api/interpreter_test.cpp)
target_link_libraries(smi_interpreter_test GTest::gtest_main smi_lib)

add_executable(smi_msdecompiler_test api/msdecompiler_test.cpp)
target_link_libraries(smi_msdecompiler_test GTest::gtest_main smi_lib)

include(GoogleTest)

gtest_discover_tests(lexer_test)
gtest_discover_tests(parser_test)

gtest_discover_tests(smi_interpreter_test)
gtest_discover_tests(smi_msdecompiler_test)
