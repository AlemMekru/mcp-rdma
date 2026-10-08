#include "mcp_rdma.h"
#include <stdlib.h>

struct mcp_rdma_context {
    int initialized;
};

mcp_rdma_context *mcp_rdma_create(void) {
    mcp_rdma_context *ctx = calloc(1, sizeof(*ctx));
    if (ctx) {
        ctx->initialized = 1;
    }
    return ctx;
}

void mcp_rdma_destroy(mcp_rdma_context *ctx) {
    free(ctx);
}

mcp_rdma_status mcp_rdma_connect(
    mcp_rdma_context *ctx,
    const char *host,
    unsigned short port
) {
    if (!ctx || !host || port == 0)
        return MCP_RDMA_ERROR;

    return MCP_RDMA_UNSUPPORTED;
}

mcp_rdma_status mcp_rdma_disconnect(
    mcp_rdma_context *ctx
) {
    if (!ctx)
        return MCP_RDMA_ERROR;

    return MCP_RDMA_UNSUPPORTED;
}