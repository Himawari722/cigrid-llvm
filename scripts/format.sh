#!/bin/bash

CLANG_FORMAT=clang-format

# 使用 LLVM 内建风格（注意是字符串，不是 YAML）
STYLE=llvm

# 要格式化的路径
TARGETS=(
  "../include"
  "../src"
)

# 遍历路径并格式化
for path in "${TARGETS[@]}"; do
  if [ -d "$path" ]; then
    find "$path" -type f \( -name "*.cpp" -o -name "*.hpp" \) -exec $CLANG_FORMAT -i --style=$STYLE {} +
  elif [ -f "$path" ]; then
    $CLANG_FORMAT -i --style=$STYLE "$path"
  fi
done

echo "所有文件已按 LLVM 风格格式化完成。"
