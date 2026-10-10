#!/usr/bin/env bash
set -euo pipefail

HOST="${1:?Usage: $0 <server-ip>}"

SERVER="./build/test_mcp_server"
CLIENT="./build/test_mcp_client"
REQUEST="tests/fixtures/server_discover_request.json"

SERVER_LOG=$(mktemp)
CLIENT_LOG=$(mktemp)
SERVER_PID=""

cleanup() {
    if [[ -n "$SERVER_PID" ]]; then
        kill "$SERVER_PID" 2>/dev/null || true
    fi
    rm -f "$SERVER_LOG" "$CLIENT_LOG"
}
trap cleanup EXIT

echo "Starting MCP-RDMA server..."
timeout 35s "$SERVER" > "$SERVER_LOG" 2>&1 &
SERVER_PID=$!

sleep 1

echo "Sending MCP JSON-RPC request..."

CLIENT_STATUS=0
timeout 15s "$CLIENT" "$HOST" "$REQUEST" \
    > "$CLIENT_LOG" 2>&1 || CLIENT_STATUS=$?

SERVER_STATUS=0
if [[ "$CLIENT_STATUS" -ne 0 ]]; then
    kill "$SERVER_PID" 2>/dev/null || true
fi
wait "$SERVER_PID" || SERVER_STATUS=$?
SERVER_PID=""

cat "$CLIENT_LOG"
cat "$SERVER_LOG"

if [[ "$CLIENT_STATUS" -ne 0 ||
      "$SERVER_STATUS" -ne 0 ]] ||
   ! grep -q "PASS: JSON-RPC response verified" "$CLIENT_LOG"; then
    echo "FAIL: MCP-RDMA integration test"
    exit 1
fi

echo "PASS: MCP-RDMA JSON-RPC request-response test"
