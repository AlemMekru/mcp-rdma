#ifndef MCP_RDMA_H
#define MCP_RDMA_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct mcp_rdma_context mcp_rdma_context;

mcp_rdma_context *mcp_rdma_create(void);

void mcp_rdma_destroy(mcp_rdma_context *ctx);

typedef enum {
    MCP_RDMA_OK = 0,
    MCP_RDMA_ERROR = -1,
    MCP_RDMA_UNSUPPORTED = -2
} mcp_rdma_status;

mcp_rdma_status mcp_rdma_connect(
    mcp_rdma_context *ctx,
    const char *host,
    unsigned short port
);

mcp_rdma_status mcp_rdma_disconnect(
    mcp_rdma_context *ctx
);

#ifdef __cplusplus
}
#endif

#endif