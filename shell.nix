{pkgs ? import <nixpkgs> {}}:
pkgs.mkShell {
  packages = with pkgs; [
    cmake
    gcc
    gbenchmark
    gtest
    llvmPackages_21.clang-tools
    util-linux
  ];

  CMAKE_TOOLCHAIN_FILE = pkgs.writeText "vectordb-toolchain.cmake" ''
    set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES
      "${pkgs.lib.getDev pkgs.gtest}/include"
      "${pkgs.lib.getDev pkgs.gbenchmark}/include"
    )
  '';
}
