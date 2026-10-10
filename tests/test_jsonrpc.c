#include <json-c/json.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern char *mcp_jsonrpc_handle(const char *request);

int main(void) {
    const char *request =
        "{\"jsonrpc\":\"2.0\",\"id\":42,"
        "\"method\":\"server/discover\"}";

    char *response = mcp_jsonrpc_handle(request);
    if (!response) {
        fprintf(stderr, "FAIL: No JSON-RPC response\n");
        return 1;
    }

    struct json_object *root = json_tokener_parse(response);
    struct json_object *id = NULL;
    struct json_object *result = NULL;
    struct json_object *versions = NULL;
    struct json_object *capabilities = NULL;

    int valid =
        root &&
        json_object_object_get_ex(root, "id", &id) &&
        json_object_is_type(id, json_type_int) &&
        json_object_get_int(id) == 42 &&
        json_object_object_get_ex(root, "result", &result) &&
        json_object_object_get_ex(result, "supportedVersions", &versions) &&
        json_object_is_type(versions, json_type_array) &&
        json_object_array_length(versions) == 1 &&
        strcmp(json_object_get_string(
            json_object_array_get_idx(versions, 0)),
            "2026-07-28") == 0 &&
        json_object_object_get_ex(result, "capabilities", &capabilities) &&
        json_object_is_type(capabilities, json_type_object);

    printf("%s: server/discover response\n",
           valid ? "PASS" : "FAIL");

    if (root) json_object_put(root);
    free(response);
    return valid ? 0 : 1;
}
