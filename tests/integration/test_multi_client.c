#include "mcp_rdma.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <server-ip>\n", argv[0]);
        return 1;
    }
    mcp_rdma_context *ctx = mcp_rdma_create();
    if (!ctx) return 1;

    mcp_rdma_status status =
        mcp_rdma_connect(ctx, argv[1], 18515);

    if (status == MCP_RDMA_OK) {
        puts("RDMA connection established.");

        const char *messages[] = {
            "Hello from MCP-RDMA",
            "Second RDMA message",
            "Third RDMA message"
        };

        for (int i = 0; i < 3; i++) {
            status = mcp_rdma_send(
                ctx, messages[i], strlen(messages[i])
            );

            if (status != MCP_RDMA_OK) break;

            printf("Sent message %d: %s\n", i + 1, messages[i]);
            fflush(stdout);

            usleep(200000);
        }
    }

    if (status != MCP_RDMA_OK)
        fprintf(stderr, "RDMA operation failed: %d\n", status);

    mcp_rdma_destroy(ctx);
    return status == MCP_RDMA_OK ? 0 : 1;
}
