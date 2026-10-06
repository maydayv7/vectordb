find_package(GTest CONFIG REQUIRED)

add_executable(vectordb_tests vector_record_test.cpp)
target_link_libraries(vectordb_tests PRIVATE vectordb GTest::gtest_main)
target_compile_options(vectordb_tests PRIVATE -Wall -Wextra -Wpedantic)

include(GoogleTest)
gtest_discover_tests(vectordb_tests DISCOVERY_MODE PRE_TEST)
