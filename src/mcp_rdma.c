#include "mcp_rdma.h"
#include <infiniband/verbs.h>
#include <rdma/rdma_cma.h>
#include <stdlib.h>
#include <netdb.h>
#include <stdio.h>
#include <poll.h>
#include <sys/socket.h>
#include <netinet/in.h>

struct mcp_rdma_context {
    struct ibv_context *device_ctx;
    struct rdma_cm_id *cm_id;
    struct rdma_cm_id *client_id;
    struct rdma_event_channel *event_channel;
    struct ibv_pd *pd;
    struct ibv_cq *cq;
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
    if (ctx->cm_id && ctx->cm_id->qp)
        rdma_destroy_qp(ctx->cm_id);
    if (ctx->cq)
        ibv_destroy_cq(ctx->cq);
    if (ctx->pd)
        ibv_dealloc_pd(ctx->pd);
    if (ctx->client_id) {
        if (ctx->client_id->qp)
            rdma_destroy_qp(ctx->client_id);
        rdma_destroy_id(ctx->client_id);
    }
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

    ctx->pd = ibv_alloc_pd(ctx->cm_id->verbs);
    if (!ctx->pd)
        return MCP_RDMA_ERROR;

    ctx->cq = ibv_create_cq(
        ctx->cm_id->verbs, 16, NULL, NULL, 0
    );

    if (!ctx->cq)
        return MCP_RDMA_ERROR;

    struct ibv_qp_init_attr qp_attr = {0};

    qp_attr.send_cq = ctx->cq;
    qp_attr.recv_cq = ctx->cq;
    qp_attr.qp_type = IBV_QPT_RC;
    qp_attr.cap.max_send_wr = 16;
    qp_attr.cap.max_recv_wr = 16;
    qp_attr.cap.max_send_sge = 1;
    qp_attr.cap.max_recv_sge = 1;

    if (rdma_create_qp(ctx->cm_id, ctx->pd, &qp_attr) != 0)
        return MCP_RDMA_ERROR;

    struct rdma_conn_param conn_param = {0};
    conn_param.responder_resources = 1;
    conn_param.initiator_depth = 1;
    conn_param.retry_count = 7;
    conn_param.rnr_retry_count = 7;

    if (rdma_connect(ctx->cm_id, &conn_param) != 0)
        return MCP_RDMA_ERROR;

    if (mcp_rdma_wait_for_event(
            ctx->event_channel,
            RDMA_CM_EVENT_ESTABLISHED) != 0)
        return MCP_RDMA_ERROR;

    return MCP_RDMA_OK;
}

mcp_rdma_status mcp_rdma_listen(
    mcp_rdma_context *ctx, unsigned short port
) {
    if (!ctx || !ctx->cm_id || port == 0)
        return MCP_RDMA_ERROR;

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);

    if (rdma_bind_addr(
            ctx->cm_id, (struct sockaddr *)&addr) != 0)
        return MCP_RDMA_ERROR;

    if (rdma_listen(ctx->cm_id, 1) != 0)
        return MCP_RDMA_ERROR;

    return MCP_RDMA_OK;
}

mcp_rdma_status mcp_rdma_accept(mcp_rdma_context *ctx) {
    if (!ctx || !ctx->event_channel || !ctx->cm_id)
        return MCP_RDMA_ERROR;

    struct pollfd fd = {
        .fd = ctx->event_channel->fd,
        .events = POLLIN
    };

    if (poll(&fd, 1, 30000) <= 0 ||
        !(fd.revents & POLLIN))
        return MCP_RDMA_ERROR;

    struct rdma_cm_event *event = NULL;

    if (rdma_get_cm_event(ctx->event_channel, &event) != 0)
        return MCP_RDMA_ERROR;

    if (event->event != RDMA_CM_EVENT_CONNECT_REQUEST ||
        event->status != 0) {
        rdma_ack_cm_event(event);
        return MCP_RDMA_ERROR;
    }

    if (ctx->client_id) {
        rdma_ack_cm_event(event);
        return MCP_RDMA_ERROR;
    }

    ctx->client_id = event->id;
    rdma_ack_cm_event(event);

    ctx->pd = ibv_alloc_pd(ctx->client_id->verbs);
    if (!ctx->pd)
        return MCP_RDMA_ERROR;

    ctx->cq = ibv_create_cq(
        ctx->client_id->verbs, 16, NULL, NULL, 0
    );
    if (!ctx->cq)
        return MCP_RDMA_ERROR;

    struct ibv_qp_init_attr qp_attr = {0};

    qp_attr.send_cq = ctx->cq;
    qp_attr.recv_cq = ctx->cq;
    qp_attr.qp_type = IBV_QPT_RC;
    qp_attr.cap.max_send_wr = 16;
    qp_attr.cap.max_recv_wr = 16;
    qp_attr.cap.max_send_sge = 1;
    qp_attr.cap.max_recv_sge = 1;

    if (rdma_create_qp(ctx->client_id, ctx->pd, &qp_attr) != 0)
        return MCP_RDMA_ERROR;

    struct rdma_conn_param conn_param = {0};
    conn_param.responder_resources = 1;
    conn_param.initiator_depth = 1;
    conn_param.retry_count = 7;
    conn_param.rnr_retry_count = 7;

    if (rdma_accept(ctx->client_id, &conn_param) != 0)
        return MCP_RDMA_ERROR;

    if (mcp_rdma_wait_for_event(
            ctx->event_channel,
            RDMA_CM_EVENT_ESTABLISHED) != 0)
        return MCP_RDMA_ERROR;

    /* RDMA connection established successfully. */
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
