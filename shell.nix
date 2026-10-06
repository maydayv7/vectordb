{pkgs ? import <nixpkgs> {}}:
pkgs.mkShell {
  packages = [
    pkgs.cmake
    pkgs.gcc
  ];
}
