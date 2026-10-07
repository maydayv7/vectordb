find_package(GTest CONFIG REQUIRED)

add_executable(vectordb_tests
    distance_test.cpp
    flat_search_test.cpp
    vector_record_test.cpp
    vector_store_io_test.cpp
)
target_link_libraries(vectordb_tests PRIVATE vectordb GTest::gtest_main)
target_compile_options(vectordb_tests PRIVATE -Wall -Wextra -Wpedantic)

include(GoogleTest)
gtest_discover_tests(vectordb_tests DISCOVERY_MODE PRE_TEST)

if(VECTORDB_TEST_SIFTSMALL)
    add_test(NAME SiftSmallDataset
        COMMAND validate_siftsmall "${PROJECT_SOURCE_DIR}/data/siftsmall"
    )
    set_tests_properties(SiftSmallDataset PROPERTIES LABELS integration)
endif()
