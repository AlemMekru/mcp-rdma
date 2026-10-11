#include <json-c/json.h>
#include <stdlib.h>
#include <string.h>

char *mcp_jsonrpc_handle(const char *request) {
    if (!request)
        return NULL;

    enum json_tokener_error parse_error = json_tokener_success;
    struct json_object *root =
        json_tokener_parse_verbose(request, &parse_error);

    if (parse_error != json_tokener_success) {
        struct json_object *response = json_object_new_object();
        struct json_object *error = json_object_new_object();

        if (!response || !error) {
            if (response) json_object_put(response);
            if (error) json_object_put(error);
            return NULL;
        }

        json_object_object_add(
            response, "jsonrpc", json_object_new_string("2.0"));
        json_object_object_add(response, "id", json_object_new_null());

        json_object_object_add(
            error, "code", json_object_new_int(-32700));
        json_object_object_add(
            error, "message", json_object_new_string("Parse error"));

        json_object_object_add(response, "error", error);

        const char *serialized =
            json_object_to_json_string_ext(
                response, JSON_C_TO_STRING_PLAIN);

        char *output = serialized ? strdup(serialized) : NULL;
        json_object_put(response);
        return output;
    }

    /* A valid JSON null is an invalid JSON-RPC request.
       Let the request validator generate error -32600. */

    struct json_object *version = NULL;
    struct json_object *method = NULL;
    struct json_object *id = NULL;

    if (!json_object_object_get_ex(root, "jsonrpc", &version) ||
        !json_object_is_type(version, json_type_string) ||
        strcmp(json_object_get_string(version), "2.0") != 0 ||
        !json_object_object_get_ex(root, "method", &method) ||
        !json_object_is_type(method, json_type_string) ||
        !json_object_object_get_ex(root, "id", &id) ||
        (json_object_get_type(id) != json_type_int &&
         json_object_get_type(id) != json_type_string)) {
        struct json_object *response = json_object_new_object();
        struct json_object *error = json_object_new_object();

        if (!response || !error) {
            if (response) json_object_put(response);
            if (error) json_object_put(error);
            json_object_put(root);
            return NULL;
        }

        json_object_object_add(
            response, "jsonrpc", json_object_new_string("2.0"));

        struct json_object *request_id = NULL;
        if (json_object_object_get_ex(root, "id", &request_id) &&
            (json_object_is_type(request_id, json_type_int) ||
             json_object_is_type(request_id, json_type_string)))
            json_object_object_add(
                response, "id", json_object_get(request_id));
        else
            json_object_object_add(
                response, "id", json_object_new_null());

        json_object_object_add(
            error, "code", json_object_new_int(-32600));
        json_object_object_add(
            error, "message",
            json_object_new_string("Invalid Request"));
        json_object_object_add(response, "error", error);

        const char *serialized =
            json_object_to_json_string_ext(
                response, JSON_C_TO_STRING_PLAIN);
        char *output = serialized ? strdup(serialized) : NULL;

        json_object_put(response);
        json_object_put(root);
        return output;
    }

    if (strcmp(json_object_get_string(method),
               "server/discover") != 0) {
        struct json_object *response = json_object_new_object();
        struct json_object *error = json_object_new_object();

        if (!response || !error) {
            if (response) json_object_put(response);
            if (error) json_object_put(error);
            json_object_put(root);
            return NULL;
        }

        json_object_object_add(
            response, "jsonrpc", json_object_new_string("2.0"));
        json_object_object_add(
            response, "id", json_object_get(id));

        json_object_object_add(
            error, "code", json_object_new_int(-32601));
        json_object_object_add(
            error, "message",
            json_object_new_string("Method not found"));

        json_object_object_add(response, "error", error);

        const char *serialized =
            json_object_to_json_string_ext(
                response, JSON_C_TO_STRING_PLAIN);

        char *output = serialized ? strdup(serialized) : NULL;

        json_object_put(response);
        json_object_put(root);
        return output;
    }

    struct json_object *response = json_object_new_object();
    struct json_object *result = json_object_new_object();
    struct json_object *versions = json_object_new_array();
    struct json_object *meta = json_object_new_object();
    struct json_object *server_info = json_object_new_object();

    json_object_object_add(
        response, "jsonrpc", json_object_new_string("2.0")
    );
    json_object_object_add(response, "id", json_object_get(id));

    json_object_array_add(
        versions, json_object_new_string("2026-07-28")
    );

    json_object_object_add(
        result, "resultType", json_object_new_string("complete")
    );
    json_object_object_add(result, "supportedVersions", versions);
    json_object_object_add(
        result, "capabilities", json_object_new_object()
    );

    json_object_object_add(
        server_info, "name", json_object_new_string("mcp-rdma")
    );
    json_object_object_add(
        server_info, "version", json_object_new_string("0.1.0")
    );
    json_object_object_add(
        meta, "io.modelcontextprotocol/serverInfo", server_info
    );
    json_object_object_add(result, "_meta", meta);
    json_object_object_add(response, "result", result);

    const char *serialized =
        json_object_to_json_string_ext(
            response, JSON_C_TO_STRING_PLAIN
        );

    char *output = serialized ? strdup(serialized) : NULL;

    json_object_put(response);
    json_object_put(root);

    return output;
}
