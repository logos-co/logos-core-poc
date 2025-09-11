{
  description = "Logos Core Environment";
  
  inputs = {
    nixpkgs.url     = "github:NixOS/nixpkgs";
    flake-utils.url = "github:numtide/flake-utils";
  };

   nixConfig = {

    substituters = [
      "https://cache.nixos.org/"
      "https://nix-community.cachix.org"
    ];

    trusted-public-keys = [
      "cache.nixos.org-1:6NCHdD59X431o0gWypbMrAURkbJ16ZPMQFGspcDShjY="
      "nix-community.cachix.org-1:mB9FSh9qf2dCimDSUo8Zy7bkq5CX+/rkCWyvRCYg3Fs="
    ];
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = import nixpkgs { inherit system; };
        commonPackages = with pkgs; [
          qt6.full   # designer, assistant, lrelease…
          cmake
          ninja
          gcc
          gdb
          ccache
          protobuf
          patchelf
          git
          which
          cargo
          rustc
          mawk
          gnumake
          pkgs."lsb-release"
          jq
          curl
          go
          vulkan-headers
          vulkan-loader
        ];
        devShell = pkgs.mkShell {
          packages = commonPackages;
          shellHook = ''
            QT_PREFIX=${pkgs.qt6.full}
            export QT_PLUGIN_PATH="$QT_PREFIX/lib/qt-6/plugins''${QT_PLUGIN_PATH:+:$QT_PLUGIN_PATH}"
            export QT_QPA_PLATFORM_PLUGIN_PATH="$QT_PREFIX/lib/qt-6/plugins/platforms"
            export QML2_IMPORT_PATH="$QT_PREFIX/lib/qt-6/qml''${QML2_IMPORT_PATH:+:$QML2_IMPORT_PATH}"
            export QML_IMPORT_PATH="$QT_PREFIX/lib/qt-6/qml''${QML_IMPORT_PATH:+:$QML_IMPORT_PATH}"
            export LD_LIBRARY_PATH="$QT_PREFIX/lib:$QT_PREFIX/lib/qt-6/lib:${pkgs.protobuf}/lib:${pkgs.vulkan-loader}/lib:${pkgs.stdenv.cc.cc.lib}/lib:${pkgs.glibc}/lib''${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
            export CMAKE_PREFIX_PATH="$QT_PREFIX''${CMAKE_PREFIX_PATH:+:$CMAKE_PREFIX_PATH}"
            git submodule update --init --recursive
          '';
        };
      in
      {
        devShells = {
          default = devShell;
        };
      });
}