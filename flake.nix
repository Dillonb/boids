{
  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";
  inputs.flake-utils.url = "github:numtide/flake-utils";

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        shortRev = with self; if sourceInfo?dirtyShortRev then sourceInfo.dirtyShortRev else sourceInfo.shortRev;
        pkgs = (import nixpkgs { inherit system; });
        tools = with pkgs; [
          cmake
          ninja
        ];
        libs = with pkgs; [
          SDL2
        ] ++ lib.optionals pkgs.stdenv.isDarwin [
          darwin.apple_sdk.frameworks.Cocoa
        ];
      in
      {
        packages.default = pkgs.stdenv.mkDerivation
          {
            pname = "boids";
            version = "0.0.1-${shortRev}";
            src = pkgs.lib.fileset.toSource {
              root = ./.;
              fileset = pkgs.lib.fileset.unions [
                ./main.c
                ./CMakeLists.txt
                ./cmake/modules/FindSDL2.cmake
              ];
            };
            buildInputs = libs;
            nativeBuildInputs = tools;
            passthru.exePath = "/bin/boids";
          };
        apps.default = {
          type = "app";
          program = "${self.packages.${system}.default}/bin/boids";
        };
        devShells.default = pkgs.mkShell {
          buildInputs = tools ++ libs;
          LD_LIBRARY_PATH = "${pkgs.vulkan-loader}/lib";
        };
      }
    );
}
