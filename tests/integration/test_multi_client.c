#include "mcp_rdma.h"
#include <stdio.h>
#include <string.h>

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

            char ack[32] = {0};
            char expected[32];
            size_t received = 0;

            snprintf(expected, sizeof(expected), "ACK %d", i + 1);

            status = mcp_rdma_receive(
                ctx, ack, sizeof(ack) - 1, &received
            );

            if (status != MCP_RDMA_OK)
                break;

            ack[received] = '\0';

            if (strcmp(ack, expected) != 0) {
                fprintf(stderr, "Unexpected ACK: %s\n", ack);
                status = MCP_RDMA_ERROR;
                break;
            }

            printf("Received: %s\n", ack);
            fflush(stdout);
        }
    }

    if (status != MCP_RDMA_OK)
        fprintf(stderr, "RDMA operation failed: %d\n", status);

    mcp_rdma_destroy(ctx);
    return status == MCP_RDMA_OK ? 0 : 1;
}
