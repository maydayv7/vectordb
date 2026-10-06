{pkgs ? import <nixpkgs> {}}:
pkgs.mkShell {
  packages = with pkgs; [
    cmake
    gcc
    gtest
    llvmPackages_21.clang-tools
  ];
}
