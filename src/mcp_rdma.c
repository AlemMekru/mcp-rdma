#include "mcp_rdma.h"
#include <infiniband/verbs.h>
#include <rdma/rdma_cma.h>
#include <stdlib.h>

struct mcp_rdma_context {
    struct ibv_context *device_ctx;
    struct rdma_cm_id *cm_id;
};

mcp_rdma_context *mcp_rdma_create(void) {
    int count = 0;
    struct ibv_device **devices = ibv_get_device_list(&count);
    if (!devices || count == 0) {
        if (devices) ibv_free_device_list(devices);
        return NULL;
    }

    mcp_rdma_context *ctx = calloc(1, sizeof(*ctx));
    if (ctx)
        ctx->device_ctx = ibv_open_device(devices[0]);

    ibv_free_device_list(devices);

    if (!ctx || !ctx->device_ctx) {
        free(ctx);
        return NULL;
    }
    if (rdma_create_id(NULL, &ctx->cm_id, NULL, RDMA_PS_TCP)) {
        ibv_close_device(ctx->device_ctx);
        free(ctx);
        return NULL;
    }

    return ctx;
}

void mcp_rdma_destroy(mcp_rdma_context *ctx) {
    if (!ctx) return;
    if (ctx->cm_id)
        rdma_destroy_id(ctx->cm_id);
    if (ctx->device_ctx)
        ibv_close_device(ctx->device_ctx);
    free(ctx);
}

mcp_rdma_status mcp_rdma_connect(
    mcp_rdma_context *ctx, const char *host, unsigned short port
) {
    if (!ctx || !host || port == 0) return MCP_RDMA_ERROR;
    return MCP_RDMA_UNSUPPORTED;
}

mcp_rdma_status mcp_rdma_disconnect(mcp_rdma_context *ctx) {
    if (!ctx) return MCP_RDMA_ERROR;
    return MCP_RDMA_UNSUPPORTED;
}
