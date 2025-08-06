# 启用 GCC 或 Clang 的彩色警告输出
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(-fcolor-diagnostics)
elseif (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    add_compile_options(-fdiagnostics-color=always)
endif ()

# 枚举出所有要编译的源文件，确保 main.cpp 一定被包含
set(SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/lexer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/diagnostics.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/parser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ast_printer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
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
