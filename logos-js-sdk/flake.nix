{
  description = "Logos JavaScript SDK with compiled logos-liblogos";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    logos-liblogos.url = "github:logos-co/logos-liblogos";
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
          
          src = ./.;
          
          nativeBuildInputs = [ 
            pkgs.nodejs 
            pkgs.python3
            pkgs.gnumake
            pkgs.gcc
            pkgs.pkg-config
          ];
          
          # Skip npm install for now - we'll handle dependencies differently
          dontBuild = true;
          
          installPhase = ''
            # Debug: Show what's in the source directory
            echo "Contents of source directory:"
            ls -la
            echo "Contents of lib directory (if exists):"
            ls -la lib/ || echo "No lib directory found"
            
            # Create the output directory
            mkdir -p $out
            
            # Copy the JavaScript SDK files
            cp -r index.js README.md package.json package-lock.json $out/
            cp -r scripts $out/
            
            # Copy node_modules if they exist (from local development)
            if [ -d "node_modules" ]; then
              cp -r node_modules $out/
              echo "Copied existing node_modules"
            else
              echo "No node_modules found - dependencies will need to be installed separately"
            fi
            
            # Create lib directory and copy the built logos-liblogos library
            mkdir -p $out/lib
            
            # Copy the library from the built logos-liblogos package
            echo "Using logos-liblogos package from GitHub: ${logosLiblogos}"
            
            # Copy libraries from the built package
            if [ -d "${logosLiblogos}/lib" ]; then
              cp -r "${logosLiblogos}/lib"/* $out/lib/
              echo "Copied libraries from ${logosLiblogos}/lib"
            fi
            
            # Copy binaries if available
            if [ -d "${logosLiblogos}/bin" ]; then
              mkdir -p $out/bin
              cp -r "${logosLiblogos}/bin"/* $out/bin/
              echo "Copied binaries from ${logosLiblogos}/bin"
            fi
            
            # Copy headers if available
            if [ -d "${logosLiblogos}/include" ]; then
              mkdir -p $out/include
              cp -r "${logosLiblogos}/include"/* $out/include/
              echo "Copied headers from ${logosLiblogos}/include"
            fi
            
            # Create a wrapper script for easy usage
            mkdir -p $out/bin
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
