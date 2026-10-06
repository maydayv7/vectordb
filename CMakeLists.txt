cmake_minimum_required(VERSION 3.20)

project(vectordb LANGUAGES CXX)

set(CMAKE_CXX_EXTENSIONS OFF)

add_library(vectordb INTERFACE)
target_include_directories(vectordb INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_compile_features(vectordb INTERFACE cxx_std_20)

add_executable(record_example examples/record_example.cpp)
target_link_libraries(record_example PRIVATE vectordb)
target_compile_options(record_example PRIVATE -Wall -Wextra -Wpedantic)
