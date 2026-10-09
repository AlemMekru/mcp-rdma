#include "mcp_rdma.h"
#include <assert.h>
#include <stdio.h>

int main(void) {
    mcp_rdma_context *ctx = mcp_rdma_create();

    assert(ctx != NULL);
    printf("RDMA device initialized successfully\n");

    mcp_rdma_destroy(ctx);
    printf("RDMA device cleanup successful\n");

    return 0;
}
