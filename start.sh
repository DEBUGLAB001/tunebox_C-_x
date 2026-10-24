#!/usr/bin/env bash
set -e

# Change to repository root
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$SCRIPT_DIR"
cd "$ROOT_DIR"

PORT="${PORT:-8080}"
export PORT

# Configure CMake if build folder does not exist
if [ ! -d "build" ]; then
    cmake -B build -S . -DCMAKE_BUILD_TYPE=Release >&2
fi

# Initial build
echo "[TuneBox] Building server..." >&2
cmake --build build --target server -j2 >&2

SERVER_BIN=""
if [ -f "./build/server.exe" ]; then
    SERVER_BIN="./build/server.exe"
elif [ -f "./build/server" ]; then
    SERVER_BIN="./build/server"
else
    echo "[TuneBox] Error: server binary not found after build." >&2
    exit 1
fi

# Track server process ID
SERVER_PID=""

cleanup() {
    echo "[TuneBox] Shutting down..." >&2
    if [ -n "$SERVER_PID" ] && kill -0 "$SERVER_PID" 2>/dev/null; then
        kill "$SERVER_PID" 2>/dev/null || true
    fi
    exit 0
}

trap cleanup SIGINT SIGTERM EXIT

# Start server in background
"$SERVER_BIN" &
SERVER_PID=$!
echo "[TuneBox] Server running with PID $SERVER_PID on port $PORT" >&2

# File watcher loop
get_mtime_sum() {
    find src include -type f \( -name "*.cpp" -o -name "*.hpp" -o -name "*.h" \) -exec stat -c %Y {} + 2>/dev/null | awk '{s+=$1} END {print s}'
}

LAST_MTIME=$(get_mtime_sum)

# If watcher cannot determine mtime, wait on server process directly
if [ -z "$LAST_MTIME" ]; then
    echo "[TuneBox] File watcher unavailable; running server in foreground." >&2
    wait "$SERVER_PID"
    exit 0
fi

echo "[TuneBox] Watching for source changes..." >&2

while true; do
    sleep 2
    # Check if server process is still alive
    if ! kill -0 "$SERVER_PID" 2>/dev/null; then
        echo "[TuneBox] Server stopped unexpectedly." >&2
        wait "$SERVER_PID"
        exit 1
    fi

    CURRENT_MTIME=$(get_mtime_sum)
    if [ "$CURRENT_MTIME" != "$LAST_MTIME" ] && [ -n "$CURRENT_MTIME" ]; then
        echo "[TuneBox] Detected changes. Rebuilding incrementally..." >&2
        if cmake --build build --target server -j2 >&2; then
            echo "[TuneBox] Build succeeded. Restarting server..." >&2
            kill "$SERVER_PID" 2>/dev/null || true
            wait "$SERVER_PID" 2>/dev/null || true
            "$SERVER_BIN" &
            SERVER_PID=$!
            LAST_MTIME="$CURRENT_MTIME"
            echo "[TuneBox] Server restarted with PID $SERVER_PID" >&2
        else
            echo "[TuneBox] Build failed! Preserving currently running server." >&2
            LAST_MTIME="$CURRENT_MTIME"
        fi
    fi
done
