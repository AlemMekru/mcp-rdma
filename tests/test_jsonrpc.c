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

    const char *unknown_request =
        "{\"jsonrpc\":\"2.0\",\"id\":99,"
        "\"method\":\"unknown/method\"}";

    char *error_response =
        mcp_jsonrpc_handle(unknown_request);

    struct json_object *error_root =
        error_response ? json_tokener_parse(error_response) : NULL;

    struct json_object *error_id = NULL;
    struct json_object *error = NULL;
    struct json_object *error_code = NULL;

    int error_valid =
        error_root &&
        json_object_object_get_ex(error_root, "id", &error_id) &&
        json_object_get_int(error_id) == 99 &&
        json_object_object_get_ex(error_root, "error", &error) &&
        json_object_is_type(error, json_type_object) &&
        json_object_object_get_ex(error, "code", &error_code) &&
        json_object_get_int(error_code) == -32601;

    printf("%s: Method not found (-32601)\\n",
           error_valid ? "PASS" : "FAIL");

    valid = valid && error_valid;

    if (error_root) json_object_put(error_root);
    free(error_response);


    /* Verify an invalid JSON-RPC request. */
    const char *invalid_request =
        "{\"jsonrpc\":\"2.0\",\"id\":42}";

    char *invalid_response =
        mcp_jsonrpc_handle(invalid_request);

    struct json_object *invalid_root =
        invalid_response ? json_tokener_parse(invalid_response) : NULL;

    struct json_object *invalid_id = NULL;
    struct json_object *invalid_error = NULL;
    struct json_object *invalid_code = NULL;

    int invalid_valid =
        invalid_root &&
        json_object_object_get_ex(invalid_root, "id", &invalid_id) &&
        json_object_is_type(invalid_id, json_type_int) &&
        json_object_get_int(invalid_id) == 42 &&
        json_object_object_get_ex(
            invalid_root, "error", &invalid_error) &&
        json_object_object_get_ex(
            invalid_error, "code", &invalid_code) &&
        json_object_is_type(invalid_code, json_type_int) &&
        json_object_get_int(invalid_code) == -32600;

    printf("%s: Invalid Request (-32600)\n",
           invalid_valid ? "PASS" : "FAIL");

    valid = valid && invalid_valid;

    if (invalid_root) json_object_put(invalid_root);
    free(invalid_response);


    /* Verify malformed JSON produces a Parse error. */
    const char *malformed_request = "{\"jsonrpc\":";

    char *parse_response =
        mcp_jsonrpc_handle(malformed_request);

    struct json_object *parse_root =
        parse_response ? json_tokener_parse(parse_response) : NULL;

    struct json_object *parse_id = NULL;
    struct json_object *parse_error = NULL;
    struct json_object *parse_code = NULL;

    int parse_valid =
        parse_root &&
        json_object_object_get_ex(parse_root, "id", &parse_id) &&
        json_object_is_type(parse_id, json_type_null) &&
        json_object_object_get_ex(
            parse_root, "error", &parse_error) &&
        json_object_object_get_ex(
            parse_error, "code", &parse_code) &&
        json_object_is_type(parse_code, json_type_int) &&
        json_object_get_int(parse_code) == -32700;

    printf("%s: Parse error (-32700)\n",
           parse_valid ? "PASS" : "FAIL");

    valid = valid && parse_valid;

    if (parse_root) json_object_put(parse_root);
    free(parse_response);


    /* Verify JSON null produces Invalid Request. */
    char *null_response = mcp_jsonrpc_handle("null");

    struct json_object *null_root =
        null_response ? json_tokener_parse(null_response) : NULL;

    struct json_object *null_id = NULL;
    struct json_object *null_error = NULL;
    struct json_object *null_code = NULL;

    int null_valid =
        null_root &&
        json_object_object_get_ex(null_root, "id", &null_id) &&
        json_object_is_type(null_id, json_type_null) &&
        json_object_object_get_ex(
            null_root, "error", &null_error) &&
        json_object_object_get_ex(
            null_error, "code", &null_code) &&
        json_object_get_int(null_code) == -32600;

    printf("%s: JSON null Invalid Request\n",
           null_valid ? "PASS" : "FAIL");

    valid = valid && null_valid;

    if (null_root) json_object_put(null_root);
    free(null_response);

    if (root) json_object_put(root);
    free(response);
    return valid ? 0 : 1;
}
