add_executable(socket_test socket_test.cpp)

target_link_libraries(socket_test GTest::gtest GTest::gtest_main)
target_include_directories(
  socket_test
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

target_compile_features(socket_test PRIVATE cxx_std_17)

include(GoogleTest)
gtest_discover_tests(socket_test)
