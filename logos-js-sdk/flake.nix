{
  description = "Logos JavaScript SDK with compiled logos-liblogos";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    logos-liblogos.url = "path:/Users/iurimatias/Projects/Logos/LogosCore/logos-liblogos";
  };

  outputs = { self, nixpkgs, logos-liblogos }:
    let
      systems = [ "aarch64-darwin" "x86_64-darwin" "aarch64-linux" "x86_64-linux" ];
      forAllSystems = f: nixpkgs.lib.genAttrs systems (system: f {
        pkgs = import nixpkgs { inherit system; };
        logosLiblogos = logos-liblogos.packages.${system}.default;
      });
    in
    {
      packages = forAllSystems ({ pkgs, logosLiblogos }: {
        default = pkgs.stdenv.mkDerivation rec {
          pname = "logos-js-sdk";
          version = "1.0.0";
          
          src = pkgs.lib.cleanSourceWith {
            src = ./.;
            filter = path: type:
              let
                base = toString ./.;
                pathStr = toString path;
                relPath = pkgs.lib.removePrefix (base + "/") pathStr;
                # Exclude node_modules and other build artifacts
                excludeNodeModules = !(pkgs.lib.hasPrefix "node_modules" relPath);
                excludeBuild = !(pkgs.lib.hasPrefix "build" relPath);
                excludeResult = !(pkgs.lib.hasPrefix "result" relPath);
              in
                pkgs.lib.cleanSourceFilter path type && excludeNodeModules && excludeBuild && excludeResult;
          };
          
          nativeBuildInputs = [ pkgs.nodejs pkgs.npm ];
          
          # Install Node.js dependencies
          preBuild = ''
            # Install npm dependencies
            npm ci --production
          '';
          
          installPhase = ''
            # Create the output directory
            mkdir -p $out
            
            # Copy the JavaScript SDK files
            cp -r index.js README.md package.json package-lock.json $out/
            cp -r scripts $out/
            
            # Copy node_modules (production dependencies only)
            if [ -d "node_modules" ]; then
              cp -r node_modules $out/
            fi
            
            # Create lib directory and copy the built logos-liblogos library
            mkdir -p $out/lib
            
            # Copy the library from the built logos-liblogos package
            # Handle different library extensions
            for lib in ${logosLiblogos}/lib/liblogos_core.*; do
              if [ -f "$lib" ]; then
                cp "$lib" $out/lib/
                echo "Copied library: $lib"
              fi
            done
            
            # Also copy any other libraries that might be needed
            if [ -d "${logosLiblogos}/bin" ]; then
              mkdir -p $out/bin
              cp -r ${logosLiblogos}/bin/* $out/bin/
            fi
            
            # Copy headers if needed
            if [ -d "${logosLiblogos}/include" ]; then
              mkdir -p $out/include
              cp -r ${logosLiblogos}/include/* $out/include/
            fi
            
            # Create a wrapper script for easy usage
            cat > $out/bin/logos-js-sdk << 'EOF'
#!/usr/bin/env bash
# Wrapper script for Logos JS SDK
export NODE_PATH="$NODE_PATH:$(dirname "$0")/../node_modules"
exec node "$(dirname "$0")/../index.js" "$@"
EOF
            chmod +x $out/bin/logos-js-sdk
          '';
          
          meta = with pkgs.lib; {
            description = "Logos JavaScript SDK with compiled logos-liblogos";
            platforms = platforms.unix;
            maintainers = [ ];
          };
        };
      });

      devShells = forAllSystems ({ pkgs }: {
        default = pkgs.mkShell {
          nativeBuildInputs = [
            pkgs.nodejs
            pkgs.npm
          ];
          
          shellHook = ''
            echo "🔧 Logos JS SDK Development Environment"
            echo "📦 Node.js version: $(node --version)"
            echo "📦 npm version: $(npm --version)"
            echo ""
            echo "Available commands:"
            echo "  npm run copy-libs  - Copy built libraries to SDK"
            echo "  npm test          - Run tests"
          '';
        };
      });
    };
}
