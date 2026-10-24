#!/usr/bin/env bash
set -e

# Change to repository root
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
cd "$ROOT_DIR"

# Configure if build directory doesn't exist
if [ ! -d "build" ]; then
    cmake -B build -S . -DCMAKE_BUILD_TYPE=Release >&2
fi

# Build challenge_tests incrementally
cmake --build build --target challenge_tests -j2 >&2

# Run test binary, stdout goes to stdout, stderr to stderr, preserving exit code
if [ -f "./build/challenge_tests.exe" ]; then
    ./build/challenge_tests.exe
elif [ -f "./build/challenge_tests" ]; then
    ./build/challenge_tests
else
    echo "{\"error\": \"challenge_tests binary not found\"}"
    exit 1
fi
