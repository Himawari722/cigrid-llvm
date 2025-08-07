#pragma once

#include <functional>
#include <string>

#include "fmt/core.h"
#include "parser/ast.hpp"

class ASTPrinter {
public:
  explicit ASTPrinter();
  // void print(const Prog &prog);

  // TODO: 分发叫dispatch吗
  void print_indent() { fmt::print("{}", std::string(current_indent, ' ')); }
  void print_ident(const std::string &name);
  void print_type(const TypeNode &ty);
  void print_expr(const ExprNode &expr);
  void print_stmt(const StmtNode &stmt);
  void print_params(const std::vector<Parameter> &params);
  void print_global(const GlobalNode &global);


  // TODO: Too complicated
  void print_list(const char *kind,
                  std::initializer_list<std::function<void()>> fields) {
    print_indent();
    fmt::print("{}(", kind);

    bool first = true;
    for (auto &f : fields) {
      if (!first) fmt::print(", ");
      first = false;
      f();            // TODO: 调用你传进来的打印 lambda
    }

    fmt::print(")\n");
  }
  
  // RAII Indent control
  struct Indent {
    ASTPrinter &printer;
    Indent(ASTPrinter &p) : printer(p) { printer.increase_indent(); }
    ~Indent() { printer.decrease_indent(); }
  };

private:

  void increase_indent() { current_indent += indent_step; };
  void decrease_indent() { current_indent -= indent_step; };



  int current_indent = 0;
  int indent_step = 2;
};