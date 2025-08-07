#pragma once

#include <optional>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "diagnostics/diagnostics.hpp"
#include "token.hpp"

class Lexer {
  bool at_eof = false;
  std::string_view source;
  int size = source.size();
  int current_pos = 0;
  char current_char = 0;
  int current_line = 1;
  int current_column = 1;
  int start_line = 1;
  int start_column = 1;
  std::vector<Token> token_list;
  Diagnostics &diag;

public:
  explicit Lexer(std::string_view source, Diagnostics &diag);
  std::vector<Token> gen_token();
  std::optional<Token> next_token();
  void print_token_list();

private:
  void next_char();
  std::optional<Token> read_char();
  std::optional<Token> read_ident_or_keyword();
  std::optional<Token> read_num();
  std::optional<Token> read_string();
  std::optional<Token> read_symbol();
  void skip_line();
  void skip_space();

private:
  static const inline std::unordered_map<std::string, TokenKind> keyword_map = {
      {"break", TokenKind::BREAK},   {"char", TokenKind::CHAR},
      {"delete", TokenKind::DELETE}, {"else", TokenKind::ELSE},
      {"extern", TokenKind::EXTERN}, {"for", TokenKind::FOR},
      {"if", TokenKind::IF},         {"int", TokenKind::INT},
      {"new", TokenKind::NEW},       {"return", TokenKind::RETURN},
      {"struct", TokenKind::STRUCT}, {"void", TokenKind::VOID},
      {"while", TokenKind::WHILE}};
};