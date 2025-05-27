#!/bin/bash

# Exit on error
set -e

echo "============================================"
echo "Building Qt Remote Objects Example (Server)"
echo "============================================"

cd server
chmod +x compile.sh
./compile.sh

echo ""
echo "============================================"
echo "Building Qt Remote Objects Example (Client)"
echo "============================================"

cd ../client
chmod +x compile.sh
./compile.sh

echo ""
echo "============================================"
echo "Build Complete!"
echo "============================================"
echo ""
echo "To run the server: cd server/build && ./counter_server"
echo "To run the client: cd client/build && ./counter_client"
echo "" 