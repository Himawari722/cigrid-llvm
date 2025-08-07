#pragma once

#include <optional>
#include <string>
#include <unordered_map>

#include "ast.hpp"
#include "common.hpp"
#include "diagnostics/diagnostics.hpp"
#include "lexer/lexer.hpp"
#include "lexer/token.hpp"

class Parser {
  CigridFlags &flags;
  Diagnostics &diag;
  std::string source;
  Lexer lexer;
  Token current_token;
  Token peek_token;
  bool has_peeked = false;

public:
  explicit Parser(std::istream &file, Diagnostics &diag, CigridFlags &flags);
  std::unique_ptr<Prog> parse();

private:
  static std::string read_stream(std::istream &file);

  void advance();
  void expect(TokenKind kind);
  void error(Position pos, std::string message);
  const Token &peek(int num = 1);

  // Return string since other others will need the name to construct
  std::string parse_ident();
  std::unique_ptr<TypeNode> parse_ty();
  bool is_type_token() const; // no arguments, just check the current token
  bool is_type_token(const Token &token) const;

  // --- Expression parsers ---
  bool is_binop() const; // no arguments, just check the current token
  bool is_binop(const TokenKind &kind) const;
  Bop parse_bop();
  Uop parse_uop();
  std::unique_ptr<ExprNode> parse_atom();
  std::unique_ptr<ExprNode> parse_expr_array_access();
  std::unique_ptr<ExprNode> parse_expr_function_call();
  std::unique_ptr<ExprNode> parse_expr_var();
  std::unique_ptr<ExprNode> parse_expr_constant();
  std::unique_ptr<ExprNode> parse_expr_unop();
  std::unique_ptr<ExprNode> parse_expr_in_paren();
  std::unique_ptr<ExprNode> parse_expr_new();
  std::unique_ptr<ExprNode> parse_expr(int min_precedence = 1);

  // --- Statement parsers ---
  std::unique_ptr<StmtNode> parse_stmt();
  std::unique_ptr<StmtNode> parse_stmt_scope();
  std::unique_ptr<StmtNode> parse_stmt_if();
  std::unique_ptr<StmtNode> parse_stmt_while();
  std::unique_ptr<StmtNode> parse_stmt_break();
  std::unique_ptr<StmtNode> parse_stmt_return();
  std::unique_ptr<StmtNode> parse_stmt_delete();
  std::unique_ptr<StmtNode> parse_stmt_for();

  // --- Assign parsers ---
  std::unique_ptr<StmtNode> parse_assign(); // need peek
  std::unique_ptr<StmtNode> parse_lvalue(); // need peek
  std::unique_ptr<StmtNode> parse_varassign();

  // --- Global parsers ---
  std::vector<Parameter> parse_params();
  std::unique_ptr<GlobalNode> parse_global();
  std::unique_ptr<GlobalNode> parse_global_extern();
  std::unique_ptr<GlobalNode> parse_global_def();
  std::unique_ptr<GlobalNode> parse_global_struct();
  std::unique_ptr<Prog> parse_prog();

private:
  static const inline std::unordered_map<TokenKind, int> precedence = {
      {TokenKind::LOGICAL_OR, 1}, {TokenKind::LOGICAL_AND, 2},
      {TokenKind::BITWISE_OR, 3}, {TokenKind::BITWISE_AND, 4},
      {TokenKind::EQUAL, 5},      {TokenKind::NOT_EQUAL, 5},
      {TokenKind::LESS_THAN, 6},  {TokenKind::LARGER_THAN, 6},
      {TokenKind::LESS_EQUAL, 6}, {TokenKind::LARGER_EQUAL, 6},
      {TokenKind::SHIFT_LEFT, 7}, {TokenKind::SHIFT_RIGHT, 7},
      {TokenKind::PLUS, 8},       {TokenKind::MINUS, 8},
      {TokenKind::MULTIPLY, 9},   {TokenKind::DIVIDE, 9},
      {TokenKind::MODULUS, 9},
      // TODO: uop not implemented yet
      // {BITWISE_NOT, ?},
      // {NOT, ?},
      // {NEG, ?},
      // {EXPONENTIAL, ?},
  };
  static const inline std::unordered_map<TokenKind, int> associativity = {
      {TokenKind::LOGICAL_OR, 1}, {TokenKind::LOGICAL_AND, 1},
      {TokenKind::BITWISE_OR, 1}, {TokenKind::BITWISE_AND, 1},
      {TokenKind::EQUAL, 1},      {TokenKind::NOT_EQUAL, 1},
      {TokenKind::LESS_THAN, 1},  {TokenKind::LARGER_THAN, 1},
      {TokenKind::LESS_EQUAL, 1}, {TokenKind::LARGER_EQUAL, 1},
      {TokenKind::SHIFT_LEFT, 1}, {TokenKind::SHIFT_RIGHT, 1},
      {TokenKind::PLUS, 1},       {TokenKind::MINUS, 1},
      {TokenKind::MULTIPLY, 1},   {TokenKind::DIVIDE, 1},
      {TokenKind::MODULUS, 1},
      // TODO: uop not implemented yet
      // {BITWISE_NOT, ?},
      // {NOT, ?},
      // {NEG, ?},
      // {EXPONENTIAL, ?},
  };

  static const inline std::unordered_map<TokenKind, Bop> bop_map = {
      {TokenKind::NOT, Bop::NOT},
      {TokenKind::BITWISE_NOT, Bop::BITWISE_NOT},
      {TokenKind::PLUS, Bop::PLUS},
      {TokenKind::MINUS, Bop::MINUS},
      {TokenKind::MULTIPLY, Bop::MULTIPLY},
      {TokenKind::DIVIDE, Bop::DIVIDE},
      {TokenKind::EXPONENTIAL, Bop::EXPONENTIAL},
      {TokenKind::MODULUS, Bop::MODULUS},
      {TokenKind::LESS_THAN, Bop::LESS_THAN},
      {TokenKind::LARGER_THAN, Bop::LARGER_THAN},
      {TokenKind::LESS_EQUAL, Bop::LESS_EQUAL},
      {TokenKind::LARGER_EQUAL, Bop::LARGER_EQUAL},
      {TokenKind::EQUAL, Bop::EQUAL},
      {TokenKind::ASSIGN, Bop::ASSIGN},
      {TokenKind::NOT_EQUAL, Bop::NOT_EQUAL},
      {TokenKind::BITWISE_AND, Bop::BITWISE_AND},
      {TokenKind::BITWISE_OR, Bop::BITWISE_OR},
      {TokenKind::LOGICAL_AND, Bop::LOGICAL_AND},
      {TokenKind::LOGICAL_OR, Bop::LOGICAL_OR},
      {TokenKind::SHIFT_LEFT, Bop::SHIFT_LEFT},
      {TokenKind::SHIFT_RIGHT, Bop::SHIFT_RIGHT},
  };

  static const inline std::unordered_map<TokenKind, Uop> uop_map = {
      {TokenKind::NOT, Uop::NOT},
      {TokenKind::BITWISE_NOT, Uop::BITWISE_NOT},
      {TokenKind::MINUS, Uop::NEG}, // unary minus
  };
};