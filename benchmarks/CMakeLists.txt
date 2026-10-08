find_package(benchmark CONFIG REQUIRED)

add_executable(flat_search_benchmark flat_search_benchmark.cpp)
target_link_libraries(flat_search_benchmark PRIVATE vectordb benchmark::benchmark)
target_compile_options(flat_search_benchmark PRIVATE -Wall -Wextra -Wpedantic)
