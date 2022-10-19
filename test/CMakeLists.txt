add_executable(socket_test socket_test.cpp)
target_link_libraries(socket_test GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(socket_test)
