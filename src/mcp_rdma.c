#include "mcp_rdma.h"
#include <infiniband/verbs.h>
#include <rdma/rdma_cma.h>
#include <stdlib.h>
#include <netdb.h>
#include <stdio.h>
#include <poll.h>
#include <sys/socket.h>

struct mcp_rdma_context {
    struct ibv_context *device_ctx;
    struct rdma_cm_id *cm_id;
    struct rdma_event_channel *event_channel;
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
    ctx->event_channel = rdma_create_event_channel();
    if (!ctx->event_channel) {
        ibv_close_device(ctx->device_ctx);
        free(ctx);
        return NULL;
    }

    if (rdma_create_id(ctx->event_channel, &ctx->cm_id, NULL, RDMA_PS_TCP)) {
        rdma_destroy_event_channel(ctx->event_channel);
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
    if (ctx->event_channel)
        rdma_destroy_event_channel(ctx->event_channel);
    if (ctx->device_ctx)
        ibv_close_device(ctx->device_ctx);
    free(ctx);
}

static int mcp_rdma_wait_for_event(
    struct rdma_event_channel *channel,
    enum rdma_cm_event_type expected
);

mcp_rdma_status mcp_rdma_connect(
    mcp_rdma_context *ctx, const char *host, unsigned short port
) {
    if (!ctx || !ctx->cm_id || !host || port == 0)
        return MCP_RDMA_ERROR;

    struct addrinfo hints = {0};
    struct addrinfo *result = NULL;
    char service[6];

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    snprintf(service, sizeof(service), "%hu", port);

    if (getaddrinfo(host, service, &hints, &result) != 0)
        return MCP_RDMA_ERROR;

    int rc = rdma_resolve_addr(
        ctx->cm_id, NULL, result->ai_addr, 2000
    );

    freeaddrinfo(result);

    if (rc != 0)
        return MCP_RDMA_ERROR;

    if (mcp_rdma_wait_for_event(
            ctx->event_channel,
            RDMA_CM_EVENT_ADDR_RESOLVED) != 0)
        return MCP_RDMA_ERROR;

    if (rdma_resolve_route(ctx->cm_id, 2000) != 0)
        return MCP_RDMA_ERROR;

    if (mcp_rdma_wait_for_event(
            ctx->event_channel,
            RDMA_CM_EVENT_ROUTE_RESOLVED) != 0)
        return MCP_RDMA_ERROR;

    return MCP_RDMA_OK;
}

mcp_rdma_status mcp_rdma_disconnect(mcp_rdma_context *ctx) {
    if (!ctx) return MCP_RDMA_ERROR;
    return MCP_RDMA_UNSUPPORTED;
}

static int mcp_rdma_wait_for_event(
    struct rdma_event_channel *channel,
    enum rdma_cm_event_type expected
) {
    struct pollfd fd = {
        .fd = channel->fd,
        .events = POLLIN
    };

    if (poll(&fd, 1, 2000) <= 0 ||
        !(fd.revents & POLLIN))
        return -1;

    struct rdma_cm_event *event = NULL;

    if (rdma_get_cm_event(channel, &event) != 0)
        return -1;

    int success = (event->event == expected &&
                   event->status == 0);

    rdma_ack_cm_event(event);
    return success ? 0 : -1;
}
