#ifndef MCP_RDMA_H
#define MCP_RDMA_H

#include <stddef.h>

#define MCP_RDMA_BUFFER_SIZE 4096

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

mcp_rdma_status mcp_rdma_listen(mcp_rdma_context *ctx, unsigned short port);
mcp_rdma_status mcp_rdma_accept(mcp_rdma_context *ctx);


mcp_rdma_status mcp_rdma_send(
    mcp_rdma_context *ctx,
    const void *data,
    size_t length
);

mcp_rdma_status mcp_rdma_receive(
    mcp_rdma_context *ctx,
    void *output,
    size_t capacity,
    size_t *received
);

#ifdef __cplusplus
}
#endif

#endif