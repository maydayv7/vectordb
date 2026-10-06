# VectorDB

A C++20 vector database.

## Build and run

Requirements: Linux, CMake 3.20+, GCC 13+ or Clang 17+, and GoogleTest.

Grab all the dependencies with Nix:

```sh
nix-shell
```

## Build configurations

```sh
cmake --preset debug
cmake --build --preset debug
ctest --preset debug
```

Replace `debug` with `tsan` or `release` to select another configuration:

| Preset    | Build directory | Purpose                    |
| --------- | --------------- | -------------------------- |
| `debug`   | `build/debug`   | Debug with ASAN, and UBSAN |
| `tsan`    | `build/tsan`    | Debug with TSAN            |
| `release` | `build/release` | Optimized code             |
