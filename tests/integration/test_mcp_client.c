#include <json-c/json.h>
#include "mcp_rdma.h"
#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <server-ip> <json-file>\n", argv[0]);
        return 1;
    }

    FILE *file = fopen(argv[2], "rb");
    if (!file) {
        perror("Cannot open JSON file");
        return 1;
    }

    char message[MCP_RDMA_BUFFER_SIZE];
    size_t length = fread(message, 1, sizeof(message), file);

    if (ferror(file) || length == 0 ||
        (length == sizeof(message) && fgetc(file) != EOF)) {
        fprintf(stderr, "Invalid or oversized JSON file\n");
        fclose(file);
        return 1;
    }

    fclose(file);

    mcp_rdma_context *ctx = mcp_rdma_create();
    if (!ctx) return 1;

    mcp_rdma_status status =
        mcp_rdma_connect(ctx, argv[1], 18515);

    if (status == MCP_RDMA_OK) {
        status = mcp_rdma_send(ctx, message, length);

        if (status == MCP_RDMA_OK)
            printf("MCP JSON-RPC request sent: %zu bytes\n", length);
    }

    if (status == MCP_RDMA_OK) {
        char response[MCP_RDMA_BUFFER_SIZE + 1] = {0};
        size_t received = 0;

        status = mcp_rdma_receive(
            ctx, response, MCP_RDMA_BUFFER_SIZE, &received
        );

        if (status == MCP_RDMA_OK) {
            response[received] = '\0';
            struct json_object *root = json_tokener_parse(response);
            struct json_object *id = NULL;
            struct json_object *result = NULL;
            struct json_object *versions = NULL;

            int valid = root &&
                json_object_object_get_ex(root, "id", &id) &&
                json_object_is_type(id, json_type_int) &&
                json_object_get_int(id) == 1 &&
                json_object_object_get_ex(root, "result", &result) &&
                json_object_is_type(result, json_type_object) &&
                json_object_object_get_ex(
                    result, "supportedVersions", &versions) &&
                json_object_is_type(versions, json_type_array) &&
                json_object_array_length(versions) > 0;

            printf("Server response: %s\\n", response);
            printf("%s\\n", valid
                ? "PASS: JSON-RPC response verified"
                : "FAIL: Invalid JSON-RPC response");

            if (!valid)
                status = MCP_RDMA_ERROR;

            if (root)
                json_object_put(root);
        }
    }

    if (status != MCP_RDMA_OK)
        fprintf(stderr, "RDMA operation failed: %d\n", status);

    mcp_rdma_destroy(ctx);
    return status == MCP_RDMA_OK ? 0 : 1;
}
