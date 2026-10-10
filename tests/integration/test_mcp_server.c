#include "mcp_rdma.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern char *mcp_jsonrpc_handle(const char *request);

int main(void) {
    mcp_rdma_context *ctx = mcp_rdma_create();
    if (!ctx) return 1;

    mcp_rdma_status status = mcp_rdma_listen(ctx, 18515);

    if (status == MCP_RDMA_OK) {
        puts("MCP-RDMA server listening...");
        fflush(stdout);
        status = mcp_rdma_accept(ctx);
    }

    if (status == MCP_RDMA_OK) {
        char request[MCP_RDMA_BUFFER_SIZE + 1] = {0};
        size_t received = 0;

        status = mcp_rdma_receive(
            ctx, request, MCP_RDMA_BUFFER_SIZE, &received
        );

        if (status == MCP_RDMA_OK) {
            request[received] = '\0';
            printf("Received JSON-RPC request: %zu bytes\n",
                   received);

            char *response = mcp_jsonrpc_handle(request);

            if (!response) {
                fprintf(stderr, "JSON-RPC processing failed\n");
                status = MCP_RDMA_ERROR;
            } else {
                size_t length = strlen(response);

                if (length > MCP_RDMA_BUFFER_SIZE)
                    status = MCP_RDMA_ERROR;
                else
                    status = mcp_rdma_send(ctx, response, length);

                if (status == MCP_RDMA_OK)
                    printf("JSON-RPC response sent: %zu bytes\n",
                           length);

                free(response);
            }
        }
    }

    if (status != MCP_RDMA_OK)
        fprintf(stderr, "RDMA operation failed: %d\n", status);

    mcp_rdma_destroy(ctx);
    return status == MCP_RDMA_OK ? 0 : 1;
}
