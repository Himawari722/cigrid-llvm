cmake_minimum_required(VERSION 3.16)
project(my_compiler LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 设置编译警告
add_compile_options(-Wall -Wextra -Wno-unused-parameter -g)

# fmt 库
add_subdirectory(external/fmt)

# 使用本地 submodule 构建好的 LLVM（静态模式）
set(LLVM_DIR "${CMAKE_SOURCE_DIR}/external/llvm-project/build/lib/cmake/llvm")
find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

# 包含 LLVM 头文件，定义 flags
include_directories(SYSTEM ${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

# 如果需要，可以根据你的环境再加：
# link_directories(${LLVM_LIBRARY_DIRS})

# 最后把 src 加进来
add_subdirectory(src)
