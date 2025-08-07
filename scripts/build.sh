#!/bin/bash
set -e

# 获取构建配置参数（Release/Debug）
build_configure=$1
shift 1

# === 路径设置 ===
ROOT_DIR=$(pwd)/..

# LLVM 配置路径
LLVM_SRC="$ROOT_DIR/external/llvm-project/llvm"
LLVM_BUILD="$ROOT_DIR/external/llvm-project/build_${build_configure}"
LLVM_INSTALL="$ROOT_DIR/external/llvm-install_${build_configure}"

# 主项目构建路径
BUILD_DIR="$ROOT_DIR/build"

# === 初始化 submodule（如果还没拉取）===
echo ">> Initializing submodules..."
git submodule update --init --recursive

# === 构建 LLVM（使用您提供的配置）===
if [ ! -f "$LLVM_INSTALL/bin/llvm-config" ]; then
  echo ">> Configuring and building LLVM with ${build_configure} configuration..."
  
  # 使用您提供的LLVM配置选项
  cmake -G Ninja \
    -S "$LLVM_SRC" \
    -B "$LLVM_BUILD" \
    -DCMAKE_BUILD_TYPE="${build_configure}" \
    -DCMAKE_INSTALL_PREFIX="${LLVM_INSTALL}" \
    -DLLVM_ENABLE_PROJECTS="" \
    -DLLVM_TARGETS_TO_BUILD="X86" \
    -DLLVM_ENABLE_DUMP=ON \
    -DLLVM_ENABLE_RTTI=ON \
    -DLLVM_INCLUDE_TESTS=OFF \
    -DLLVM_INCLUDE_DOCS=OFF \
    -DLLVM_INCLUDE_BENCHMARKS=OFF \
    -DLLVM_INCLUDE_EXAMPLES=OFF \
    -DLLVM_ENABLE_ABI_BREAKING_CHECKS=ON \
    -DLLVM_OPTIMIZED_TABLEGEN=ON \
    -DBUILD_SHARED_LIBS=OFF \
    "$@"

  # cmake --build "$LLVM_BUILD" --target install -- -j$(nproc)
  # 强制使用 8 个进程
  # cmake --build "$LLVM_BUILD" --target install -- -j8
  # 强制使用 4 个进程
  # cmake --build "$LLVM_BUILD" --target install -- -j4
  # 强制使用 2 个进程
  cmake --build "$LLVM_BUILD" --target install -- -j2
  # 强制使用 1 个进程
  # cmake --build "$LLVM_BUILD" --target install -- -j1

  echo ">> LLVM built and installed to $LLVM_INSTALL"
else
  echo ">> LLVM already installed at $LLVM_INSTALL"
fi

# === 构建主项目 ===
echo ">> Configuring and building your project with ${build_configure} configuration..."

cmake -G Ninja \
  -S "$ROOT_DIR" \
  -B "$BUILD_DIR" \
  -DCMAKE_BUILD_TYPE="${build_configure}" \
  -DCMAKE_INSTALL_PREFIX="$BUILD_DIR/install" \
  -DCMAKE_PREFIX_PATH="$LLVM_INSTALL"

cmake --build "$BUILD_DIR" -- -j$(nproc)

echo "Build finished: see ${BUILD_DIR}/"