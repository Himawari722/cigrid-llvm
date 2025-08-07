#include <cstdlib> // use for std::exit
#include <istream>
#include <variant>

#include "common.hpp"
#include "fmt/core.h"
#include "lexer/lexer.hpp"
#include "lexer/token.hpp"
#include "parser/parser.hpp"

/* Grammars:
| "<<" | ">>" (5)    TODO: need implementation
*/

// A temporary function, may not need it in submission
std::string to_string(TokenKind kind) {
  switch (kind) {
  // Operators
  case TokenKind::NOT:
    return "NOT";
  case TokenKind::BITWISE_NOT:
    return "BITWISE_NOT";
  case TokenKind::PLUS:
    return "PLUS";
  case TokenKind::MINUS:
    return "MINUS";
  case TokenKind::MULTIPLY:
    return "MULTIPLY";
  case TokenKind::DIVIDE:
    return "DIVIDE";
  case TokenKind::EXPONENTIAL:
    return "EXPONENTIAL";
  case TokenKind::MODULUS:
    return "MODULUS";
  case TokenKind::LESS_THAN:
    return "LESS_THAN";
  case TokenKind::LARGER_THAN:
    return "LARGER_THAN";
  case TokenKind::LESS_EQUAL:
    return "LESS_EQUAL";
  case TokenKind::LARGER_EQUAL:
    return "LARGER_EQUAL";
  case TokenKind::EQUAL:
    return "EQUAL";
  case TokenKind::ASSIGN:
    return "ASSIGN";
  case TokenKind::NOT_EQUAL:
    return "NOT_EQUAL";
  case TokenKind::BITWISE_AND:
    return "BITWISE_AND";
  case TokenKind::BITWISE_OR:
    return "BITWISE_OR";
  case TokenKind::LOGICAL_AND:
    return "LOGICAL_AND";
  case TokenKind::LOGICAL_OR:
    return "LOGICAL_OR";
  case TokenKind::SHIFT_LEFT:
    return "SHIFT_LEFT";
  case TokenKind::SHIFT_RIGHT:
    return "SHIFT_RIGHT";

  // Separators
  case TokenKind::LPAREN:
    return "LPAREN";
  case TokenKind::RPAREN:
    return "RPAREN";
  case TokenKind::LBRACKET:
    return "LBRACKET";
  case TokenKind::RBRACKET:
    return "RBRACKET";
  case TokenKind::LBRACE:
    return "LBRACE";
  case TokenKind::RBRACE:
    return "RBRACE";
  case TokenKind::SEMICOLON:
    return "SEMICOLON";
  case TokenKind::COMMA:
    return "COMMA";
  case TokenKind::PERIOD:
    return "PERIOD";

  // Keywords
  case TokenKind::CHAR:
    return "CHAR";
  case TokenKind::INT:
    return "INT";
  case TokenKind::VOID:
    return "VOID";
  case TokenKind::BREAK:
    return "BREAK";
  case TokenKind::DELETE:
    return "DELETE";
  case TokenKind::ELSE:
    return "ELSE";
  case TokenKind::EXTERN:
    return "EXTERN";
  case TokenKind::FOR:
    return "FOR";
  case TokenKind::IF:
    return "IF";
  case TokenKind::NEW:
    return "NEW";
  case TokenKind::RETURN:
    return "RETURN";
  case TokenKind::STRUCT:
    return "STRUCT";
  case TokenKind::WHILE:
    return "WHILE";

  // Constants & Identifiers
  case TokenKind::IDENTIFIER:
    return "IDENTIFIER";
  case TokenKind::INT_LITERAL:
    return "INT_LITERAL";
  case TokenKind::CHAR_LITERAL:
    return "CHAR_LITERAL";
  case TokenKind::STRING_LITERAL:
    return "STRING_LITERAL";

  // Miscellaneous
  case TokenKind::END_OF_FILE:
    return "END_OF_FILE";
  case TokenKind::BAD:
    return "BAD";

  // 安全处理未覆盖的情况（编译器通常会警告缺失的枚举值）
  default:
    return "UNKNOWN_TOKEN";
  }
}

Parser::Parser(std::istream &file, Diagnostics &diag, CigridFlags &flags)
    : flags(flags), diag(diag), source(read_stream(file)), lexer(source, diag) {

  advance();
  if (flags.debug) {
    fmt::print("Parser initialized.\n");
    fmt::print("The first token is: {}\n", current_token.lexeme);
  }
}
auto Parser::read_stream(std::istream &file) -> std::string {
  return std::string((std::istreambuf_iterator<char>(file)),
                     std::istreambuf_iterator<char>());
}

auto Parser::parse() -> std::unique_ptr<Prog> {
  auto prog = parse_prog();
  expect(TokenKind::END_OF_FILE);
  return prog;
}

auto Parser::advance() -> void {
  if (has_peeked) {
    current_token = peek_token;
    has_peeked = false;
  } else
    current_token =
        lexer.next_token().value(); // next_token returns std::optional<Token>
  if (flags.debug) {
    // TODO: a temp debug print
    fmt::print("Advanced to token: {}\n", current_token.lexeme);
  }
  if (current_token.kind == TokenKind::BAD) {
    error(current_token.pos,
          fmt::format("bad token encountered, {}", current_token.lexeme));
  }
}

auto Parser::expect(TokenKind kind) -> void {
  if (current_token.kind == kind) {
    advance();
  } else {
    // TODO: a temp debug print, cannot print the name of TokenKind but int
    // need implementation of fmt::format for TokenKind, or implement a
    // to_string method for TokenKind
    // TODO: what is the difference between enum and enum class?
    error(current_token.pos,
          fmt::format("Expected token kind {}, but got {}", to_string(kind),
                      to_string(current_token.kind)));
  }
}

auto Parser::error(Position pos, std::string message) -> void {
  // diag.error(pos, message);
  if (flags.line_error) {
    fmt::print(stderr, "{}", pos.line);
  }
  if (flags.debug) {
    fmt::print(stderr, "Error at {}:{}: {}\n", pos.line, pos.column, message);
  }
  // TODO: a better way to handle errors
  std::exit(1);
}

auto Parser::peek(int num) -> const Token & {
  if (!has_peeked && num == 1) {
    peek_token = lexer.next_token().value();
    has_peeked = true;
  }
  return peek_token;
}

auto Parser::parse_ident() -> std::string {
  auto pos = current_token.pos;
  // TODO: 这个bug记录一下：
  //   bug 原因是调用 advance() 后访问了 std::variant 中被覆盖的内部指针，导致返回的标识符字符串变成悬垂引用并打印出乱码。
  // 补充建议：先 std::move 提取字符串再调用 advance() 可避免该问题
  // GPT找到这一段，还有总结好的bug详细解析
  // 也还有点没完全搞懂

  // if (auto *name = std::get_if<std::string>(&current_token.value)) {
  //   advance();
  //   return *name;
  // } 
  if (std::holds_alternative<std::string>(current_token.value)){
    std::string ident = std::move(std::get<std::string>(current_token.value));
    advance();
    return ident;
  } else {
    error(pos, "fail to parse identifier token");
  }
  return ""; // unreachable, but needed to satisfy the return type
}
auto Parser::parse_ty() -> std::unique_ptr<TypeNode> {
  auto pos = current_token.pos;
  std::unique_ptr<TypeNode> result;
  switch (current_token.kind) {
  case TokenKind::VOID:
    advance();
    result = std::make_unique<TypeNode>(TVoid{pos});
    break;
  case TokenKind::INT:
    advance();
    result = std::make_unique<TypeNode>(TInt{pos});
    break;
  case TokenKind::CHAR:
    advance();
    result = std::make_unique<TypeNode>(TChar{pos});
    break;
  case TokenKind::IDENTIFIER: {
    if (auto *name = std::get_if<std::string>(&current_token.value)) {
      result = std::make_unique<TypeNode>(TIdent{pos, *name});
      advance();
      break;
    } else {
      error(pos, fmt::format("Expected an identifier for type, but got {}",
                             to_string(current_token.kind)));
    }
  }
  default:
    error(pos, fmt::format("Expected a type token, but got {}",
                           to_string(current_token.kind)));
  }
  while (current_token.kind == TokenKind::MULTIPLY) {
    pos = current_token.pos;
    result = std::make_unique<TypeNode>(TPoint{pos, std::move(result)});
    advance();
  }

  return result;
}
auto Parser::is_type_token() const -> bool {
  using enum TokenKind;
  switch (current_token.kind) {
  case CHAR:
  case INT:
  case VOID:
  case IDENTIFIER:
    return true;

  default:
    return false;
  }
};

auto Parser::is_type_token(const Token &token) const -> bool {
  using enum TokenKind;
  switch (token.kind) {
  case CHAR:
  case INT:
  case VOID:
  case IDENTIFIER:
    return true;

  default:
    return false;
  }
}

auto Parser::is_binop() const -> bool {
  return precedence.find(current_token.kind) != precedence.end();
}

auto Parser::is_binop(const TokenKind &kind) const -> bool {
  return precedence.find(kind) != precedence.end();
}

auto Parser::parse_bop() -> Bop {
  // Map current TokenKind to ast Bop
  auto sth = bop_map.find(current_token.kind);
  if (sth != bop_map.end()) {
    advance();
    return sth->second;
  } else {
    error(current_token.pos,
          fmt::format("Expected a binary operator, but got {}",
                      to_string(current_token.kind)));
  }

  return Bop::NOT; // unreachable, but needed to satisfy the return type
}

auto Parser::parse_uop() -> Uop {
  // Map current TokenKind to ast Uop
  auto sth = uop_map.find(current_token.kind);
  if (sth != uop_map.end()) {
    advance();
    return sth->second;
  } else {
    error(current_token.pos,
          fmt::format("Expected a unary operator, but got {}",
                      to_string(current_token.kind)));
  }

  return Uop::NOT; // unreachable, but needed to satisfy the return type
}

// expr → Ident | UInt | Char | String (7)
// | expr binop expr (8)
// | unop expr (9)
// | Ident "(" [ expr { "," expr } ] ")" (10)
// | "new" ty "[" expr "]" (11)
// | "(" expr ")" (13)
// expr is parsed as atom()
auto Parser::parse_atom() -> std::unique_ptr<ExprNode> {
  if (current_token.kind == TokenKind::IDENTIFIER) {
    if (peek(1).kind == TokenKind::LBRACKET) {
      // array access
      return parse_expr_array_access();
    } else if (peek(1).kind == TokenKind::LPAREN) {
      // function call
      return parse_expr_function_call();
    } else {
      // variable
      return parse_expr_var();
    }
  }
  if (current_token.kind == TokenKind::INT_LITERAL ||
      current_token.kind == TokenKind::CHAR_LITERAL ||
      current_token.kind == TokenKind::STRING_LITERAL) {
    // constant
    return parse_expr_constant();
  }
  if (current_token.kind == TokenKind::NOT ||
      current_token.kind == TokenKind::BITWISE_NOT ||
      current_token.kind == TokenKind::MINUS) {
    // unop
    return parse_expr_unop();
  }

  if (current_token.kind == TokenKind::LPAREN) {
    // parenthesized expression
    return parse_expr_in_paren();
  }
  if (current_token.kind == TokenKind::NEW) {
    return parse_expr_new();
  } else {
    error(current_token.pos, fmt::format("Expected an expression, but got {}",
                                         to_string(current_token.kind)));
  }

  return nullptr; // unreachable, but needed to satisfy the return type
}

// expr → UInt | Char | String (7)
auto Parser::parse_expr_constant() -> std::unique_ptr<ExprNode> {
  auto pos = current_token.pos;
  auto token_value_visitor = overload{
      [=, this](int value) {
        advance();
        return std::make_unique<ExprNode>(EInt{pos, value});
      },
      [=, this](char value) {
        advance();
        return std::make_unique<ExprNode>(EChar{pos, value});
      },
      [=, this](std::string value) {
        advance();
        return std::make_unique<ExprNode>(EString{pos, std::move(value)});
      },
      // default case
      [=, this](auto &&) -> std::unique_ptr<ExprNode> {
        error(pos, "unsupported token type");
        return nullptr; // unreachable, but needed to satisfy the return type
      }};
  // Note: current_token.value is a std::variant<std::monostate, int, char,
  // std::string> The visitor will handle the different types and return the
  // appropriate ExprNode type.
  return std::visit(token_value_visitor, current_token.value);
}

// expr → Ident (7)
auto Parser::parse_expr_var() -> std::unique_ptr<ExprNode> {
  // use get_if to access variant
  if (auto *name = std::get_if<std::string>(&current_token.value)) {
    advance();
    return std::make_unique<ExprNode>(EVar{current_token.pos, *name});
  }
  // There should be no case where current_token.value is not a string here,
  // since we already checked that current_token.kind is IDENTIFIER.
  return nullptr; // unreachable, but needed to satisfy the return type
}

// | unop expr (9)
auto Parser::parse_expr_unop() -> std::unique_ptr<ExprNode> {
  auto pos = current_token.pos;
  auto op = parse_uop();
  auto rhs = parse_atom();
  return std::make_unique<ExprNode>(EUnOp{pos, op, std::move(rhs)});
}

// | Ident "(" [ expr { "," expr } ] ")" (10)
auto Parser::parse_expr_function_call() -> std::unique_ptr<ExprNode> {
  auto pos = current_token.pos;
  auto name = parse_ident();
  expect(TokenKind::LPAREN);
  std::vector<std::unique_ptr<ExprNode>> args;
  if (current_token.kind != TokenKind::RPAREN) {
    args.push_back(parse_expr(1));
    while (current_token.kind == TokenKind::COMMA) {
      advance();
      args.push_back(parse_expr(1)); // parse next argument
    }
  }
  expect(TokenKind::RPAREN);
  return std::make_unique<ExprNode>(
      ECall{pos, std::move(name), std::move(args)});
}

// | "new" ty "[" expr "]" (11)
auto Parser::parse_expr_new() -> std::unique_ptr<ExprNode> {
  auto pos = current_token.pos;
  advance();
  auto type = parse_ty();
  expect(TokenKind::LBRACKET);
  auto index = parse_expr(1);
  expect(TokenKind::RBRACKET);
  return std::make_unique<ExprNode>(
      ENew{pos, std::move(type), std::move(index)});
}

// | Ident "[" expr "]" ["." Ident] (12)
auto Parser::parse_expr_array_access() -> std::unique_ptr<ExprNode> {
  auto pos = current_token.pos;
  auto name = parse_ident();
  expect(TokenKind::LBRACKET);
  auto index = parse_expr(1);
  expect(TokenKind::RBRACKET);
  std::optional<std::string> label;
  if (current_token.kind == TokenKind::PERIOD) {
    advance();
    label = parse_ident();
  }
  return std::make_unique<ExprNode>(
      EArrayAccess{pos, std::move(name), std::move(index), std::move(label)});
}

// | "(" expr ")" (13)
auto Parser::parse_expr_in_paren() -> std::unique_ptr<ExprNode> {
  advance();
  auto expr = parse_expr(1);
  expect(TokenKind::RPAREN);
  return expr;
}

auto Parser::parse_expr(int min_precedence) -> std::unique_ptr<ExprNode> {
  auto pos = current_token.pos;
  auto lhs = parse_atom();
  while (true) {
    if (!is_binop() || precedence.at(current_token.kind) < min_precedence) {
      break;
    }
    auto prec = precedence.at(current_token.kind);
    auto assoc = associativity.at(current_token.kind);
    auto op = parse_bop();
    auto rhs = parse_expr(prec + assoc);
    lhs = std::make_unique<ExprNode>(
        EBinOp{pos, op, std::move(lhs), std::move(rhs)});
  }
  return lhs;
}

// stmt → varassign ";" (14)
// | "{" { stmt } "}" (15)
// | "if" "(" expr ")" stmt [ "else" stmt ] (16)
// | "while" "(" expr ")" stmt (17)
// | "break" ";" (18)
// | "return" [ expr ] ";" (19)
// | "delete" "[" "]" Ident ";" (20)
// | "for" "(" varassign ";" expr ";" assign ")" stmt (21)
auto Parser::parse_stmt() -> std::unique_ptr<StmtNode> {
  if (current_token.kind == TokenKind::LBRACE) {
    return parse_stmt_scope();
  }
  if (current_token.kind == TokenKind::IF) {
    return parse_stmt_if();
  }
  if (current_token.kind == TokenKind::WHILE) {
    return parse_stmt_while();
  }
  if (current_token.kind == TokenKind::BREAK) {
    return parse_stmt_break();
  }
  if (current_token.kind == TokenKind::RETURN) {
    return parse_stmt_return();
  }
  if (current_token.kind == TokenKind::DELETE) {
    return parse_stmt_delete();
  }
  if (current_token.kind == TokenKind::FOR) {
    return parse_stmt_for();
  } else {
    // parse varassign
    auto varassign = parse_varassign();
    expect(TokenKind::SEMICOLON);
    return varassign;
  }
}

// | "{" { stmt } "}" (15)
auto Parser::parse_stmt_scope() -> std::unique_ptr<StmtNode> {
  auto pos = current_token.pos;
  advance(); // consume LBRACE
  std::vector<std::unique_ptr<StmtNode>> stmts;
  while (current_token.kind != TokenKind::RBRACE) {
    stmts.push_back(parse_stmt());
  }
  expect(TokenKind::RBRACE);
  return std::make_unique<StmtNode>(SScope{pos, std::move(stmts)});
}

// | "if" "(" expr ")" stmt [ "else" stmt ] (16)
auto Parser::parse_stmt_if() -> std::unique_ptr<StmtNode> {
  auto pos = current_token.pos;
  advance(); // consume IF
  expect(TokenKind::LPAREN);
  auto cond = parse_expr(1);
  expect(TokenKind::RPAREN);
  auto then_branch = parse_stmt();
  std::unique_ptr<StmtNode> else_branch = nullptr;
  if (current_token.kind == TokenKind::ELSE) {
    advance(); // consume ELSE
    else_branch = parse_stmt();
  }
  return std::make_unique<StmtNode>(SIf{
      pos, std::move(cond), std::move(then_branch), std::move(else_branch)});
}

// | "while" "(" expr ")" stmt (17)
auto Parser::parse_stmt_while() -> std::unique_ptr<StmtNode> {
  auto pos = current_token.pos;
  advance(); // consume WHILE
  expect(TokenKind::LPAREN);
  auto cond = parse_expr(1);
  expect(TokenKind::RPAREN);
  auto stmt = parse_stmt();
  return std::make_unique<StmtNode>(
      SWhile{pos, std::move(cond), std::move(stmt)});
}

// | "break" ";" (18)
auto Parser::parse_stmt_break() -> std::unique_ptr<StmtNode> {
  auto pos = current_token.pos;
  advance(); // consume BREAK
  expect(TokenKind::SEMICOLON);
  return std::make_unique<StmtNode>(SBreak{pos});
}

// | "return" [ expr ] ";" (19)
auto Parser::parse_stmt_return() -> std::unique_ptr<StmtNode> {
  auto pos = current_token.pos;
  advance(); // consume RETURN
    std::unique_ptr<ExprNode> expr = nullptr;
    if (current_token.kind != TokenKind::SEMICOLON) {
      expr = parse_expr(1);
    }
    expect(TokenKind::SEMICOLON);
    return std::make_unique<StmtNode>(SReturn{pos, std::move(expr)});
}

// | "delete" "[" "]" Ident ";" (20)
auto Parser::parse_stmt_delete() -> std::unique_ptr<StmtNode> {
  auto pos = current_token.pos;
  advance(); // consume DELETE
  expect(TokenKind::LBRACKET);
  expect(TokenKind::RBRACKET);
  auto name = parse_ident();
  expect(TokenKind::SEMICOLON);
  return std::make_unique<StmtNode>(SDelete{pos, std::move(name)});
}

// | "for" "(" varassign ";" expr ";" assign ")" stmt (21)
auto Parser::parse_stmt_for() -> std::unique_ptr<StmtNode> {
  auto pos = current_token.pos;
  advance(); // consume FOR
  expect(TokenKind::LPAREN);
  auto varassign = parse_varassign();
  expect(TokenKind::SEMICOLON);
  auto cond = parse_expr(1);
  expect(TokenKind::SEMICOLON);
  auto assign = parse_assign();
  expect(TokenKind::RPAREN);
  auto stmt = parse_stmt();

  auto stmts_and_update = std::vector<std::unique_ptr<StmtNode>>();
  stmts_and_update.push_back(std::move(stmt));
  stmts_and_update.push_back(std::move(assign));
  auto s_body =
      std::make_unique<StmtNode>(SScope{pos, std::move(stmts_and_update)});
  auto s_while = std::make_unique<StmtNode>(
      SWhile{pos, std::move(cond), std::move(s_body)});
  auto s_for = std::vector<std::unique_ptr<StmtNode>>();
  s_for.push_back(std::move(varassign));
  s_for.push_back(std::move(s_while));
  return std::make_unique<StmtNode>(SScope{pos, std::move(s_for)});
}

// lvalue → Ident | Ident "[" expr "]" [ "." Ident ] (22)
// assign → lvalue "=" expr | lvalue "++" | lvalue "--" (24)
// This function not only parse the lvalue, but also return the assign
// expression
auto Parser::parse_lvalue() -> std::unique_ptr<StmtNode> {
  auto pos = current_token.pos;
  auto name = parse_ident();
  if (current_token.kind == TokenKind::LBRACKET) {
    // assign to an array elements
    advance(); // consume LBRACKET
    auto index = parse_expr(1);
    expect(TokenKind::RBRACKET);
    std::optional<std::string> label;
    if (current_token.kind == TokenKind::PERIOD) {
      advance(); // consume PERIOD
      if (current_token.kind != TokenKind::IDENTIFIER) {
        error(current_token.pos,
              fmt::format("Expected Identifier after '.', but got {}",
                          to_string(current_token.kind)));
      }
      label = parse_ident();
    }
    // assign to this array element
    if (current_token.kind == TokenKind::ASSIGN) {
      advance();
      auto value = parse_expr(1);
      return std::make_unique<StmtNode>(
          SArrayAssign{pos, std::move(name), std::move(index), std::move(label),
                       std::move(value)});
    }

    // ++/-- when when the lvalue is an array element
    else if (current_token.kind == TokenKind::PLUS) {
      advance();
      expect(TokenKind::PLUS);
      return std::make_unique<StmtNode>(SArrayPlusAssign{
          pos, std::move(name), std::move(index), std::move(label),
          std::make_unique<ExprNode>(EInt{pos, 1})});
    }

    else if (current_token.kind == TokenKind::MINUS) {
      advance();
      expect(TokenKind::MINUS);
      return std::make_unique<StmtNode>(SArrayMinusAssign{
          pos, std::move(name), std::move(index), std::move(label),
          std::make_unique<ExprNode>(EInt{pos, 1})});
    }

    else {
      error(current_token.pos,
            fmt::format("Expected '=', '++' or '--' after lvalue, but got {}",
                        to_string(current_token.kind)));
    }
  } else {
    // assign to a variable (lvalue is just a ident)
    if (current_token.kind == TokenKind::ASSIGN) {
      advance();
      auto value = parse_expr(1);
      return std::make_unique<StmtNode>(
          SVarAssign{pos, std::move(name), std::move(value)});
    }

    // ident ++/--
    else if (current_token.kind == TokenKind::PLUS) {
      advance();
      expect(TokenKind::PLUS);
      auto bin_op = std::make_unique<ExprNode>(
          EBinOp{pos, Bop::PLUS, std::make_unique<ExprNode>(EVar{pos, name}),
                 std::make_unique<ExprNode>(EInt{pos, 1})});
      return std::make_unique<StmtNode>(
          SVarAssign{pos, name, std::move(bin_op)});
    }

    else if (current_token.kind == TokenKind::MINUS) {
      advance();
      expect(TokenKind::MINUS);
      auto bin_op = std::make_unique<ExprNode>(
          EBinOp{pos, Bop::MINUS, std::make_unique<ExprNode>(EVar{pos, name}),
                 std::make_unique<ExprNode>(EInt{pos, 1})});
      return std::make_unique<StmtNode>(
          SVarAssign{pos, name, std::move(bin_op)});
    }

    else {
      error(current_token.pos,
            fmt::format("Expected '=', '++' or '--' after lvalue, but got {}",
                        to_string(current_token.kind)));
    }
  }

  return nullptr; // unreachable, but needed to satisfy the return type
}

// assign → Ident "(" [ expr { "," expr } ] ")" (23)
// | lvalue "=" expr | lvalue "++" | lvalue "--" (24)
auto Parser::parse_assign() -> std::unique_ptr<StmtNode> {
  auto pos = current_token.pos;
  if (current_token.kind != TokenKind::IDENTIFIER) {
    error(current_token.pos,
          fmt::format("Expected Identifier to be assigned, but got {}",
                      to_string(current_token.kind)));
    return nullptr; // unreachable, but needed to satisfy the return type
  } else {
    if (peek(1).kind == TokenKind::LPAREN) {
      // function call
      auto name = parse_ident();
      advance(); // consume LPAREN
      std::vector<std::unique_ptr<ExprNode>> args;
      if (current_token.kind != TokenKind::RPAREN) {
        args.push_back(parse_expr(1));
        while (current_token.kind == TokenKind::COMMA) {
          advance();
          args.push_back(parse_expr(1));
        }
      }
      expect(TokenKind::RPAREN);
      return std::make_unique<StmtNode>(
          SExpr{pos, std::make_unique<ExprNode>(
                         ECall{pos, std::move(name), std::move(args)})});
    } else {
      return parse_lvalue();
    }
  }
}

// varassign → ty Ident "=" expr | assign (25)
auto Parser::parse_varassign() -> std::unique_ptr<StmtNode> {
  auto pos = current_token.pos;
  // varassign starts with ty, all assign starts with Ident, but Ident is a
  // part of ty, so use peek to check if the token after ty is Idnet
  // note that ty can be TPoint, so peek(1) may be * instead of IDENTIFIER
  if (is_type_token()) {
    if (peek(1).kind == TokenKind::IDENTIFIER ||
        peek(1).kind == TokenKind::MULTIPLY) {
      // varassign
      auto type = parse_ty();
      auto name = parse_ident();
      expect(TokenKind::ASSIGN);
      auto value = parse_expr(1);
      return std::make_unique<StmtNode>(
          SVarAssign{pos, std::move(name), std::move(value)});
    } else {
      return parse_assign();
    }
  } else {
    error(current_token.pos,
          fmt::format("Expected type token or Identifier, but got {}",
                      to_string(current_token.kind)));
  }
  return nullptr; // unreachable, but needed to satisfy the return type
}

// params → [ ty Ident { "," ty Ident } ] (26)
auto Parser::parse_params() -> std::vector<Parameter> {
  auto result = std::vector<Parameter>();
  if (is_type_token()) {
    result.push_back(Parameter{parse_ty(), parse_ident()});
    while (current_token.kind == TokenKind::COMMA) {
      advance();
      result.push_back(Parameter{parse_ty(), parse_ident()});
    }
  }
  return result;
}

// global → ty Ident "(" params ")" "{" { stmt } "}" (27)
// | "extern" ty Ident "(" params ")" ";" (28)
// | ty Ident "=" expr ";" (29)
// | "extern" ty Ident ";" (30)
// | "struct" Ident "{" { ty Ident ";" } "}" ";" (31)
auto Parser::parse_global() -> std::unique_ptr<GlobalNode> {
  if (current_token.kind == TokenKind::STRUCT) {
    return parse_global_struct();
  }

  else if (current_token.kind == TokenKind::EXTERN) {
    return parse_global_extern();
  }

  else if (is_type_token()) {
    return parse_global_def();
  } else {
    error(current_token.pos,
          fmt::format("Expected 'struct', 'extern' or type token, but got {}",
                      to_string(current_token.kind)));
  }
  return nullptr; // unreachable, but needed to satisfy the return type
}

auto Parser::parse_global_extern() -> std::unique_ptr<GlobalNode> {
  auto pos = current_token.pos;
  advance();
  auto type = parse_ty();
  auto name = parse_ident();

  if (current_token.kind == TokenKind::LPAREN) {
    // extern function declaration
    advance();
    auto params = parse_params();
    expect(TokenKind::RPAREN);
    expect(TokenKind::SEMICOLON);
    return std::make_unique<GlobalNode>(
        GFuncDecl{pos, std::move(type), std::move(name), std::move(params)});
  }

  if (current_token.kind == TokenKind::SEMICOLON) {
    // extern variable declaration
    advance();
    return std::make_unique<GlobalNode>(
        GVarDecl{pos, std::move(type), std::move(name)});
  }

  error(current_token.pos, fmt::format("Expected ';' or '(', but got {}'",
                                       to_string(current_token.kind)));
  return nullptr; // unreachable, but needed to satisfy the return type
}

auto Parser::parse_global_def() -> std::unique_ptr<GlobalNode> {
  auto pos = current_token.pos;
  auto type = parse_ty();
  auto name = parse_ident();

  if (current_token.kind == TokenKind::LPAREN) {
    // function definition
    advance();
    auto params = parse_params();
    ;
    expect(TokenKind::RPAREN);
    expect(TokenKind::LBRACE);
    auto stmt_pos = current_token.pos;
    std::vector<std::unique_ptr<StmtNode>> stmts;
    while (current_token.kind != TokenKind::RBRACE) {
      stmts.push_back(parse_stmt());
    }
    expect(TokenKind::RBRACE);
    return std::make_unique<GlobalNode>(GFuncDef{
        pos, std::move(type), std::move(name), std::move(params),
        std::make_unique<StmtNode>(SScope{stmt_pos, std::move(stmts)})});
  }

  if (current_token.kind == TokenKind::ASSIGN) {
    // variable definition
    advance();
    auto value = parse_expr(1);
    expect(TokenKind::SEMICOLON);
    return std::make_unique<GlobalNode>(
        GVarDef{pos, std::move(type), std::move(name), std::move(value)});
  }

  error(current_token.pos, fmt::format("Expected '(', '=', but got {}",
                                       to_string(current_token.kind)));
  return nullptr; // unreachable, but needed to satisfy the return type
}

auto Parser::parse_global_struct() -> std::unique_ptr<GlobalNode> {
  auto pos = current_token.pos;
  advance();
  auto name = parse_ident();
  expect(TokenKind::LBRACE);

  std::vector<Parameter> params;
  if (is_type_token()) {
    params.push_back(Parameter{parse_ty(), parse_ident()});
    expect(TokenKind::SEMICOLON);
    while (current_token.kind != TokenKind::RBRACE) {
      params.push_back(Parameter{parse_ty(), parse_ident()});
      expect(TokenKind::SEMICOLON);
    }
  }
  expect(TokenKind::RBRACE);
  expect(TokenKind::SEMICOLON);

  return std::make_unique<GlobalNode>(GStruct{pos, name, std::move(params)});
}

auto Parser::parse_prog() -> std::unique_ptr<Prog> {
  auto prog = std::make_unique<Prog>();
  while (current_token.kind != TokenKind::END_OF_FILE) {
    prog->globals.push_back(std::move(parse_global()));
  }
  return prog;
}