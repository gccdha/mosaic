{ pkgs ? import <nixpkgs> {} }:
pkgs.mkShell {
    nativeBuildInputs = with pkgs; [
    pkg-config
    gdb
    feh
    cmake
    opencv
    perf
  ];
}
