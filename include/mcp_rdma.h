#ifndef MCP_RDMA_H
#define MCP_RDMA_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct mcp_rdma_context mcp_rdma_context;

mcp_rdma_context *mcp_rdma_create(void);

void mcp_rdma_destroy(mcp_rdma_context *ctx);

#ifdef __cplusplus
}
#endif

#endif