#include "mcp_rdma.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <server-ip>\n", argv[0]);
        return 1;
    }
    mcp_rdma_context *ctx = mcp_rdma_create();
    if (!ctx) return 1;

    mcp_rdma_status status =
        mcp_rdma_connect(ctx, argv[1], 18515);

    if (status == MCP_RDMA_OK) {
        puts("RDMA connection established.");

        const char *message = "RDMA-BENCH";
        FILE *csv = fopen("benchmarks/rdma_rtt.csv", "w");
        if (!csv) {
            perror("Cannot create benchmark CSV");
            mcp_rdma_destroy(ctx);
            return 1;
        }
        fprintf(csv, "iteration,rtt_us\n");

        for (int i = 0; i < 1100; i++) {
            struct timespec start, end;
            clock_gettime(CLOCK_MONOTONIC, &start);
            status = mcp_rdma_send(
                ctx, message, strlen(message)
            );

            if (status != MCP_RDMA_OK) break;


            char ack[32] = {0};
            char expected[32];
            size_t received = 0;

            snprintf(expected, sizeof(expected), "ACK %d", i + 1);

            status = mcp_rdma_receive(
                ctx, ack, sizeof(ack) - 1, &received
            );

            clock_gettime(CLOCK_MONOTONIC, &end);
            if (status != MCP_RDMA_OK)
                break;

            ack[received] = '\0';

            if (strcmp(ack, expected) != 0) {
                fprintf(stderr, "Unexpected ACK: %s\n", ack);
                status = MCP_RDMA_ERROR;
                break;
            }

            if (i >= 100) {
                double us = (end.tv_sec - start.tv_sec) * 1e6
                          + (end.tv_nsec - start.tv_nsec) / 1e3;
                fprintf(csv, "%d,%.3f\n", i - 100, us);
            }
        }
        if (fclose(csv) != 0)
            status = MCP_RDMA_ERROR;
    }

    if (status != MCP_RDMA_OK)
        fprintf(stderr, "RDMA operation failed: %d\n", status);

    mcp_rdma_destroy(ctx);
    return status == MCP_RDMA_OK ? 0 : 1;
}
