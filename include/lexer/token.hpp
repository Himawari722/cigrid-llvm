#pragma once

#include <string>
#include <variant>

#include "common.hpp"

enum class TokenKind {
  // --- Operators ---
  NOT,
  BITWISE_NOT,
  PLUS,
  MINUS,
  MULTIPLY,
  DIVIDE,
  EXPONENTIAL,
  MODULUS,
  LESS_THAN,
  LARGER_THAN,
  LESS_EQUAL,
  LARGER_EQUAL,
  EQUAL,
  ASSIGN,
  NOT_EQUAL,
  BITWISE_AND,
  BITWISE_OR,
  LOGICAL_AND,
  LOGICAL_OR,
  SHIFT_LEFT,
  SHIFT_RIGHT,

  // --- Separators ---
  LPAREN,
  RPAREN,
  LBRACKET,
  RBRACKET,
  LBRACE,
  RBRACE,
  SEMICOLON,
  COMMA,
  PERIOD,

  // --- Keywords ---
  CHAR,
  INT,
  VOID,
  BREAK,
  DELETE,
  ELSE,
  EXTERN,
  FOR,
  IF,
  NEW,
  RETURN,
  STRUCT,
  WHILE,

  // Constants and Identifiers
  IDENTIFIER,
  INT_LITERAL,
  CHAR_LITERAL,
  STRING_LITERAL,

  // --- Miscellaneous ---
  END_OF_FILE,
  BAD
};

struct Token {
  TokenKind kind;
  std::string lexeme;
  Position pos;
  std::variant<std::monostate, int, char, std::string> value;
};