#include "mcp_rdma.h"
#include <stdio.h>

int main(void) {
    mcp_rdma_context *ctx = mcp_rdma_create();
    if (!ctx) return 1;

    mcp_rdma_status status = mcp_rdma_listen(ctx, 18515);

    if (status == MCP_RDMA_OK) {
        puts("Server listening on port 18515...");
        fflush(stdout);
        status = mcp_rdma_accept(ctx);
    }

    if (status == MCP_RDMA_OK) {
        puts("RDMA connection established.");
        fflush(stdout);

        for (int i = 0; i < 3; i++) {
            char message[MCP_RDMA_BUFFER_SIZE + 1] = {0};
            size_t received = 0;

            status = mcp_rdma_receive(
                ctx, message, MCP_RDMA_BUFFER_SIZE, &received
            );

            if (status != MCP_RDMA_OK) break;

            message[received] = '\0';
            printf("Message %d: %s\n", i + 1, message);
            fflush(stdout);
        }
    }

    if (status != MCP_RDMA_OK)
        fprintf(stderr, "RDMA operation failed: %d\n", status);

    mcp_rdma_destroy(ctx);
    return status == MCP_RDMA_OK ? 0 : 1;
}
