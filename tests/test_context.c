#include "mcp_rdma.h"
#include <assert.h>
#include <stdio.h>

int main(void) {
    mcp_rdma_context *ctx = mcp_rdma_create();
    assert(ctx != NULL);

    mcp_rdma_destroy(ctx);

    printf("All tests passed!\n");
    return 0;
}