#!/usr/bin/env bash
set -euo pipefail

HOST="${1:?Usage: $0 <server-ip>}"

SERVER="./build/test_multi_server"
CLIENT="./build/test_multi_client"

LOG=$(mktemp)
SERVER_PID=""

cleanup() {
    if [[ -n "$SERVER_PID" ]]; then
        kill "$SERVER_PID" 2>/dev/null || true
    fi
    rm -f "$LOG"
}
trap cleanup EXIT

echo "Starting RDMA server..."
timeout 40s "$SERVER" > "$LOG" 2>&1 &
SERVER_PID=$!

sleep 1

echo "Starting RDMA client..."
CLIENT_STATUS=0
"$CLIENT" "$HOST" || CLIENT_STATUS=$?

SERVER_STATUS=0
wait "$SERVER_PID" || SERVER_STATUS=$?
SERVER_PID=""

echo "----- Server output -----"
cat "$LOG"

if [[ "$CLIENT_STATUS" -ne 0 || "$SERVER_STATUS" -ne 0 ]]; then
    echo "RDMA integration test FAILED"
    exit 1
fi

# Verify the exact messages received by the server.
expected=(
    "Message 1: Hello from MCP-RDMA"
    "Message 2: Second RDMA message"
    "Message 3: Third RDMA message"
)

for message in "${expected[@]}"; do
    if ! grep -Fxq -- "$message" "$LOG"; then
        echo "FAILED: Missing or incorrect message: $message"
        exit 1
    fi
done

echo "RDMA integration test PASSED: all 3 messages verified"
