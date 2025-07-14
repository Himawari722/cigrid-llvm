# 枚举出所有要编译的源文件，确保 main.cpp 一定被包含
set(SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/lexer/lexer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/diagnostics/diagnostics.cpp
    # …如果后面又加了新模块，记得在这里加
)

add_executable(cigrid ${SOURCES})

target_include_directories(cigrid PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

# 链接 fmt
target_link_libraries(cigrid PRIVATE fmt::fmt)

# 拾取并链接LLVM静态组件
llvm_map_components_to_libnames(LLVM_LIBS
    support
    core
    irreader
    # 如有需要，可继续添加 passmanager、analysis、transformutils 等
)

target_link_libraries(cigrid
    PRIVATE
        ${LLVM_LIBS}
)
