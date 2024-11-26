{
  description = "Use HSLS Shaders to render a wallpaper/desktop background. Supports Wayland compositors.";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = {
    nixpkgs,
    flake-utils,
    ...
  }:
    flake-utils.lib.eachDefaultSystem (
      system: let
        # Grab Packages
        pkgs = nixpkgs.legacyPackages.${system};
        package_name = "shaderbg";
        # Required at build time
        nativeBuildInputs = [
          pkgs.meson
          pkgs.ninja
          pkgs.cmake
          pkgs.pkg-config

          pkgs.libGLU
          pkgs.wayland
          pkgs.wayland-scanner
        ];
      in {
        packages = rec {
          shaderbg = pkgs.stdenv.mkDerivation {
            src = ./.;
            name = "${package_name}";
            version = "0.0.1";
            inherit nativeBuildInputs;
            configurePhase = ''
              meson setup builddir && cd builddir
            '';
            buildPhase = ''
              meson compile
            '';
            testPhase = ''
              meson test
            '';
            installPhase = ''
              mkdir -p $out/bin
              cp ${package_name} $out/bin/
            '';
          };
          default = shaderbg;
        };
      }
    );
}
