cmake_minimum_required(VERSION 3.20)

project(vectordb LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(vectordb INTERFACE)
target_include_directories(vectordb INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_compile_features(vectordb INTERFACE cxx_std_20)

set(VECTORDB_SANITIZER "none" CACHE STRING "Sanitizer: none, address-undefined, or thread")
set_property(CACHE VECTORDB_SANITIZER PROPERTY STRINGS none address-undefined thread)

if(VECTORDB_SANITIZER STREQUAL "address-undefined")
    target_compile_options(vectordb INTERFACE
        -fsanitize=address,undefined
        -fno-omit-frame-pointer
        -fno-sanitize-recover=all
    )
    target_link_options(vectordb INTERFACE -fsanitize=address,undefined)
elseif(VECTORDB_SANITIZER STREQUAL "thread")
    target_compile_options(vectordb INTERFACE
        -fsanitize=thread
        -fno-omit-frame-pointer
    )
    target_link_options(vectordb INTERFACE -fsanitize=thread)
elseif(NOT VECTORDB_SANITIZER STREQUAL "none")
    message(FATAL_ERROR "Unknown VECTORDB_SANITIZER '${VECTORDB_SANITIZER}'.")
endif()

add_executable(record_example examples/record_example.cpp)
target_link_libraries(record_example PRIVATE vectordb)
target_compile_options(record_example PRIVATE -Wall -Wextra -Wpedantic)

add_executable(validate_siftsmall examples/validate_siftsmall.cpp)
target_link_libraries(validate_siftsmall PRIVATE vectordb)
target_compile_options(validate_siftsmall PRIVATE -Wall -Wextra -Wpedantic)

add_custom_target(fetch-siftsmall
    COMMAND "${CMAKE_COMMAND}" -P "${PROJECT_SOURCE_DIR}/cmake/DownloadSiftSmall.cmake"
    VERBATIM
)

include(CTest)
option(VECTORDB_TEST_SIFTSMALL "Test the downloaded SIFT-small dataset" OFF)
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

option(VECTORDB_BUILD_BENCHMARKS "Build release benchmarks" OFF)
if(VECTORDB_BUILD_BENCHMARKS)
    if(NOT CMAKE_BUILD_TYPE STREQUAL "Release" OR NOT VECTORDB_SANITIZER STREQUAL "none")
        message(FATAL_ERROR "Benchmarks require Release with VECTORDB_SANITIZER=none")
    endif()
    add_subdirectory(benchmarks)
endif()

include(cmake/DevTools.cmake)
