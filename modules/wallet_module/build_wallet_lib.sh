#!/bin/bash

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
VENDOR_DIR="$SCRIPT_DIR/vendor/go-wallet-sdk"
LIB_DIR="$SCRIPT_DIR/lib"

if [ ! -d "$VENDOR_DIR" ]; then
  echo "Cloning go-wallet-sdk (branch compile_c_lib) into vendor..."
  mkdir -p "$SCRIPT_DIR/vendor"
  git clone --branch compile_c_lib https://github.com/status-im/go-wallet-sdk.git "$VENDOR_DIR"
fi

echo "Building C library via make build-c-lib..."
cd "$VENDOR_DIR"
make build-c-lib

mkdir -p "$LIB_DIR"

# Attempt to copy typical outputs
if [ -f "$VENDOR_DIR/build/libgowalletsdk.so" ]; then
  cp "$VENDOR_DIR/build/libgowalletsdk.so" "$LIB_DIR/libgowalletsdk.so"
fi
if [ -f "$VENDOR_DIR/build/libgowalletsdk.dylib" ]; then
  cp "$VENDOR_DIR/build/libgowalletsdk.dylib" "$LIB_DIR/libgowalletsdk.dylib"
fi
if [ -f "$VENDOR_DIR/build/libgowalletsdk.dll" ]; then
  cp "$VENDOR_DIR/build/libgowalletsdk.dll" "$LIB_DIR/libgowalletsdk.dll"
fi
if [ -f "$VENDOR_DIR/build/libgowalletsdk.h" ]; then
  cp "$VENDOR_DIR/build/libgowalletsdk.h" "$LIB_DIR/libgowalletsdk.h"
fi

echo "Wallet C library copied into $LIB_DIR"


