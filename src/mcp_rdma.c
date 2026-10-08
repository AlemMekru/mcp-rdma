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