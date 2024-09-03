{
  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";
  inputs.flake-utils.url = "github:numtide/flake-utils";

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = (import nixpkgs { inherit system; });
        tools = with pkgs; [
          cmake
          ninja
          gdb
        ];
        libs = with pkgs; [
          SDL2
        ];
      in
      {
        devShells.default = pkgs.mkShell {
          buildInputs = tools ++ libs;
          LD_LIBRARY_PATH = "${pkgs.vulkan-loader}/lib";
        };
      }
    );
}
