cmake_minimum_required(VERSION 3.16)

project(mcp_rdma VERSION 0.1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(mcp_rdma STATIC
    src/mcp_rdma.c
)

target_include_directories(mcp_rdma PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

enable_testing()

add_executable(test_context tests/test_context.c)
target_link_libraries(mcp_rdma PUBLIC ibverbs rdmacm)
target_link_libraries(test_context PRIVATE mcp_rdma)

add_test(NAME context_test COMMAND test_context)
add_executable(test_rdma_device tests/test_rdma_device.c)
target_link_libraries(test_rdma_device PRIVATE mcp_rdma)
add_test(NAME rdma_device_test COMMAND test_rdma_device)


# RDMA client-server integration test executables
add_executable(test_multi_server
    tests/integration/test_multi_server.c
)
target_link_libraries(test_multi_server PRIVATE mcp_rdma)

add_executable(test_multi_client
    tests/integration/test_multi_client.c
)
target_link_libraries(test_multi_client PRIVATE mcp_rdma)


# Optional RDMA integration test
set(MCP_RDMA_TEST_HOST "" CACHE STRING
    "IP address of the RDMA integration test server")

if(MCP_RDMA_TEST_HOST)
    add_test(
        NAME rdma_multi_message_test
        COMMAND bash
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/integration/run_multi_test.sh
            ${MCP_RDMA_TEST_HOST}
    )

    set_tests_properties(rdma_multi_message_test PROPERTIES
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        TIMEOUT 45
    )
endif()
