#!/bin/bash
set -e

# 路径设置
ROOT_DIR=$(pwd)
LLVM_SRC="$ROOT_DIR/external/llvm-project"
LLVM_BUILD="$LLVM_SRC/build"
LLVM_INSTALL="$LLVM_SRC/install"
BUILD_DIR="$ROOT_DIR/build"

# 初始化 submodule（如果还没拉取）
echo "📦 Initializing submodules..."
git submodule update --init --recursive

# 构建 LLVM（只编译 llvm core）
if [ ! -d "$LLVM_BUILD" ]; then
  echo "🔨 Building LLVM..."
  mkdir -p "$LLVM_BUILD"
  cd "$LLVM_BUILD"
  cmake -G Ninja ../llvm \
  -DLLVM_ENABLE_PROJECTS="llvm" \
  -DLLVM_BUILD_LLVM_DYLIB=OFF \
  -DCMAKE_BUILD_TYPE=Release \
  -DLLVM_TARGETS_TO_BUILD="host" \
  -DCMAKE_INSTALL_PREFIX="$LLVM_INSTALL"

  ninja
  ninja install
  echo "✅ LLVM built and installed to $LLVM_INSTALL"
else
  echo "✅ LLVM already built."
fi

# 回到根目录，构建你的主项目
echo "🚧 Configuring and building project..."
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"
cmake -G Ninja .. -DLLVM_DIR="$LLVM_INSTALL/lib/cmake/llvm"
ninja

echo "🎉 Build finished: ./build/my_compiler"
