#!/usr/bin/env bash
set -e
cd "$(dirname "$0")"

# 1. Configure and perform initial build if not present
if [ ! -d build ]; then
    cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
fi
cmake --build build --target server -j2 1>&2

# Determine server binary path
SERVER_BIN="./build/server"
if [ -f "./build/server.exe" ]; then
    SERVER_BIN="./build/server.exe"
fi

# 2. Start server in background
PORT=8080 "$SERVER_BIN" &
PID=$!

# Clean up daemon on script exit
trap 'kill -TERM "$PID" 2>/dev/null; exit 0' SIGTERM SIGINT EXIT

# Snapshot timestamps of all source & header files
get_snap() {
    find src include -type f \( -name "*.cpp" -o -name "*.hpp" -o -name "*.h" \) -exec stat -c "%Y %n" {} + 2>/dev/null | md5sum | cut -d' ' -f1
}
LAST_SNAP=$(get_snap)

# 3. Hot-reload polling loop
while true; do
    sleep 2
    CUR_SNAP=$(get_snap)
    if [ "$CUR_SNAP" != "$LAST_SNAP" ]; then
        echo "[Engine] Source modified. Rebuilding C++ server..."
        # Incrementally compile ONLY the server target
        if cmake --build build --target server -j2 1>&2; then
            kill -TERM "$PID" 2>/dev/null || true
            wait "$PID" 2>/dev/null || true
            PORT=8080 "$SERVER_BIN" &
            PID=$!
            echo "[Engine] Server reloaded successfully."
        fi
        LAST_SNAP="$CUR_SNAP"
    fi
done
