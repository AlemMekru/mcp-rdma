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