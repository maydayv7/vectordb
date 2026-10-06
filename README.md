# VectorDB

A C++20 vector database.

## Build and run

Requirements: Linux, CMake 3.20+, GCC 13+ or Clang 17+, and GoogleTest.

Grab all the dependencies with Nix:

```sh
nix-shell
```

## Debug

```sh
cmake -S . -B build/debug -DCMAKE_BUILD_TYPE=Debug
cmake --build build/debug
./build/debug/record_example
```

## Run tests

After building:

```sh
ctest --test-dir build/debug --output-on-failure
```
