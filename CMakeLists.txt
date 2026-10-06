cmake_minimum_required(VERSION 3.20)

project(vectordb LANGUAGES CXX)

set(CMAKE_CXX_EXTENSIONS OFF)

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

include(CTest)
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
