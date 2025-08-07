#include <string>
#include <variant>  
#include <vector>

#include "common.hpp"
#include "fmt/core.h"
#include "parser/ast.hpp"
#include "printer/ast_printer.hpp"

ASTPrinter::ASTPrinter() : current_indent(0), indent_step(2) {}

// auto ASTPrinter::print(const Prog &prog) -> void {
//   for (const auto &global : prog.globals) {
//     print_global(*global);
//   }
// } 

auto ASTPrinter::print_type(const TypeNode &type) -> void {
  std::visit([this](auto &node) {
      node.print(*this);
    }, type);
}

auto ASTPrinter::print_expr(const ExprNode &expr) -> void {
  std::visit([this](auto &node) {
      node.print(*this);
    }, expr);
}

auto ASTPrinter::print_stmt(const StmtNode &stmt) -> void {
  std::visit([this](auto &node) {
      node.print(*this);
    }, stmt);
}

auto ASTPrinter::print_params(const std::vector<Parameter> &params) -> void {
  fmt::print("{{");
  for (const auto &param : params) {
    param.print(*this);
  }
  fmt::print("}}");
}

auto ASTPrinter::print_global(const GlobalNode &global) -> void {
  std::visit([this](auto &node) {
      node.print(*this);
    }, global);
  fmt::print("\n\n");
}

auto TVoid::print(ASTPrinter &p) const -> void {
  fmt::print("TVoid");
}

auto TInt::print(ASTPrinter &p) const -> void {
  fmt::print("TInt");
}

auto TChar::print(ASTPrinter &p) const -> void {
  fmt::print("TChar");
}

auto TIdent::print(ASTPrinter &p) const -> void {
  fmt::print("TIdent(\"{}\")", name);
}

auto TPoint::print(ASTPrinter &p) const -> void {
  fmt::print("TPoint(");
  p.print_type(*point_type);
  fmt::print(")");
}

auto EVar::print(ASTPrinter &p) const -> void {
  fmt::print("EVar(\"{}\")", name);
}

auto EInt::print(ASTPrinter &p) const -> void {
  fmt::print("EInt({})", value);
}

auto EChar::print(ASTPrinter &p) const -> void {
  fmt::print("EChar(");
  // TODO: to be furthre simplified using map
  switch (value) {
    case '\\':
      fmt::print("'\\\\'");
      break;
    case '\n':
      fmt::print("'\\n'");
      break;
    case '\t':
      fmt::print("'\\t'");
      break;
    case '\'':
      fmt::print("'\\''");
      break;
    case '\"':
      fmt::print("'\\\"'");
      break;
    default:
      fmt::print("'{}'", value);
      break;
  }
}

auto EString::print(ASTPrinter &p) const -> void {
  // The question mark quoted the string with special characters escaped
  fmt::print("EString({:?})", value);
}

auto EBinOp::print(ASTPrinter &p) const -> void {
  fmt::print("EBinOp(");
  // TODO: to be furthre simplified using map
  switch (op) {
    case Bop::PLUS:
      fmt::print("+");
      break;
    case Bop::MINUS:
      fmt::print("-");
      break;
    case Bop::MULTIPLY:
      fmt::print("*");
      break;
    case Bop::DIVIDE:
      fmt::print("/");
      break;
    case Bop::MODULUS:
      fmt::print("%");
      break;
    case Bop::LESS_THAN:
      fmt::print("<");
      break;
    case Bop::LARGER_THAN:
      fmt::print(">");
      break;
    case Bop::LESS_EQUAL:
      fmt::print("<=");
      break;
    case Bop::LARGER_EQUAL:
      fmt::print(">=");
      break;
    case Bop::EQUAL:
      fmt::print("==");
      break;
    case Bop::NOT_EQUAL:
      fmt::print("!=");
      break;
    case Bop::BITWISE_AND:
      fmt::print("&");
      break;
    case Bop::BITWISE_OR:
      fmt::print("|");
      break;
    case Bop::LOGICAL_AND:
      fmt::print("&&");
      break;
    case Bop::LOGICAL_OR:
      fmt::print("||");
      break;
    case Bop::SHIFT_LEFT:
      fmt::print("<<");
      break;
    case Bop::SHIFT_RIGHT:
      fmt::print(">>");
      break;
  }
  fmt::print(", ");
  p.print_expr(*lhs);
  fmt::print(", ");
  p.print_expr(*rhs);
  fmt::print(")");
}

auto EUnOp::print(ASTPrinter &p) const -> void {
  fmt::print("EUnOp(");
  switch (op) {
    case Uop::NEG:
      fmt::print("-");
      break;
    case Uop::NOT:
      fmt::print("!");
      break;
    case Uop::BITWISE_NOT:
      fmt::print("~");
      break;
  }
  fmt::print(", ");
  p.print_expr(*rhs);
  fmt::print(")");
}

auto ECall::print(ASTPrinter &p) const -> void {
  fmt::print("ECall(");
  fmt::print("\"{}\", ", name);
  fmt::print("{{");
  for (const auto &arg : args) {
    p.print_expr(*arg);
  }
  fmt::print("}}");
  fmt::print(")");
}

auto ENew::print(ASTPrinter &p) const -> void {
  fmt::print("ENew(");
  p.print_type(*type);
  fmt::print(", ");
  p.print_expr(*expr);
  fmt::print(")");
}

auto EArrayAccess::print(ASTPrinter &p) const -> void {
  fmt::print("EArrayAccess(");
  fmt::print("\"{}\", ", name);
  p.print_expr(*index);
  if (label) {
    fmt::print(", \"{}\"", *label);
  }
  fmt::print(")");
}

auto SExpr::print(ASTPrinter &p) const -> void {
  p.print_indent();
  fmt::print("SExpr(");
  p.print_expr(*expr);
  fmt::print(")");
}

auto SVarDef::print(ASTPrinter &p) const -> void {
  p.print_indent();
  fmt::print("SVarDef(");
  p.print_type(*type);
  fmt::print(", \"{}\", ", name);
  p.print_expr(*value);
  fmt::print(")");
}

auto SVarAssign::print(ASTPrinter &p) const -> void {
  p.print_indent();
  fmt::print("SVarAssign(");
  fmt::print("\"{}\", ", name);
  p.print_expr(*value);
  fmt::print(")");
}

auto SArrayAssign::print(ASTPrinter &p) const -> void {
  p.print_indent();
  fmt::print("SArrayAssign(");
  fmt::print("\"{}\", ", name);
  p.print_expr(*index);
  fmt::print(", ");
  if (label) {
    fmt::print("\"{}\", ", *label);
  }
  p.print_expr(*value);
  fmt::print(")");
}

auto SArrayPlusAssign::print(ASTPrinter &p) const -> void {
  p.print_indent();
  fmt::print("SArrayAssign(");
  fmt::print("\"{}\", ", name);
  p.print_expr(*index);
  fmt::print(", ");
  if (label) {
    fmt::print("\"{}\", ", *label);
  }
  // The expression here will be EInt(1). Here we need to print it with EBinOp and EAarryAccess
  fmt::print("EBinOp(+, EArrayAccess(\"{}\", ", name);
  p.print_expr(*index);  
  fmt::print(", EInt(1))");
  fmt::print(")");
}

auto SArrayMinusAssign::print(ASTPrinter &p) const -> void {
  p.print_indent();
  fmt::print("SArrayAssign(");
  fmt::print("\"{}\", ", name);
  p.print_expr(*index);
  fmt::print(", ");
  if (label) {
    fmt::print("\"{}\", ", *label);
  }
  // The expression here will be EInt(1). Here we need to print it with EBinOp and EAarryAccess
  fmt::print("EBinOp(-, EArrayAccess(\"{}\", ", name);
  p.print_expr(*index);  
  fmt::print(", EInt(1))");
  fmt::print(")");
}

auto SScope::print(ASTPrinter &p) const -> void {
  p.print_indent();
  fmt::print("SScope({{");
  {
    ASTPrinter::Indent indent(p);
    for (const auto &stmt : stmts) {
      fmt::print("\n");
      p.print_stmt(*stmt);
    }
  }
  fmt::print("\n");
  p.print_indent();
  fmt::print("}})");
}

auto SIf::print(ASTPrinter &p) const -> void {
  p.print_indent();
  fmt::print("SIf(");
  p.print_expr(*cond);
  fmt::print(", ");
  p.print_stmt(*then_branch);
  if (else_branch) {
    fmt::print(", ");
    p.print_stmt(*else_branch);
  }
  fmt::print(")");
}

auto SWhile::print(ASTPrinter &p) const -> void {
  p.print_indent();
  fmt::print("SWhile(");
  p.print_expr(*cond);
  fmt::print(", ");
  p.print_stmt(*stmt);
  fmt::print(")");
}

auto SBreak::print(ASTPrinter &p) const -> void {
  p.print_indent();
  fmt::print("SBreak");
}

auto SReturn::print(ASTPrinter &p) const -> void {
  p.print_indent();
  fmt::print("SReturn(");
  if (expr) {
    p.print_expr(*expr);
  }
  fmt::print(")");
}

auto SDelete::print(ASTPrinter &p) const -> void {
  p.print_indent();
  fmt::print("SDelete({})", name);
}

auto Parameter::print(ASTPrinter &p) const -> void {
  fmt::print("(");
  p.print_type(*type);
  fmt::print(", {})", name);
}

auto GFuncDef::print(ASTPrinter &p) const -> void {
  p.print_list("GFuncDef", {
    [&]{p.print_type(*return_type);},
    [&]{fmt::print("{}", name);},
    [&]{p.print_params(params);},
    [&]{
      fmt::print("\n");
      ASTPrinter::Indent indent(p);
      p.print_stmt(*stmt);
     },
  });
}

auto GFuncDecl::print(ASTPrinter &p) const -> void {
  p.print_list("GFuncDecl", {
    [&]{p.print_type(*return_type);},
    [&]{fmt::print("{}", name);},
    [&]{p.print_params(params);},
  });
}

auto GVarDef::print(ASTPrinter &p) const -> void {
  p.print_list("GVarDef", {
    [&]{p.print_type(*type);},
    [&]{fmt::print("{}", name);},
    [&]{p.print_expr(*value);},
  });
}

auto GVarDecl::print(ASTPrinter &p) const -> void {
  p.print_list("GVarDecl", {
    [&]{p.print_type(*type);},
    [&]{fmt::print("{}", name);}
  });
}

auto GStruct::print(ASTPrinter &p) const -> void {
  p.print_list("GStruct", {
    [&]{ fmt::print("{}", name); },
    [&]{ p.print_params(fields); },
  });
}

auto Prog::print(ASTPrinter &p) const -> void {
  for (const auto &global : globals) {
    p.print_global(*global);
  }
}