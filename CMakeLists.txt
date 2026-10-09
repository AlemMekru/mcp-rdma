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
target_link_libraries(mcp_rdma PUBLIC ibverbs)
target_link_libraries(test_context PRIVATE mcp_rdma)

add_test(NAME context_test COMMAND test_context)
add_executable(test_rdma_device tests/test_rdma_device.c)
target_link_libraries(test_rdma_device PRIVATE mcp_rdma)
add_test(NAME rdma_device_test COMMAND test_rdma_device)
