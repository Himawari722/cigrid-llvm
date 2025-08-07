#include "lexer/lexer.hpp"
#include "fmt/core.h"

#include <cctype>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

Lexer::Lexer(std::string_view input, Diagnostics &diag)
    : source(input), diag(diag) {
  next_char();
}

std::vector<Token> Lexer::gen_token() {
  while (auto token_option = next_token()) {
    auto token = token_option.value();
    token_list.push_back(token);
    // token_list.push_back(std::move(token));
    if (token.kind == TokenKind::END_OF_FILE) {
      break;
    }
    if (token.kind == TokenKind::BAD) {
      diag.error(token.pos, "bad token encountered");
      break;
    }
  }
  return token_list;
}

void Lexer::print_token_list() {
  for (const auto &token : token_list) {
    // Only suppor single file here
    fmt::print(
        "{:<20}{}\n",
        fmt::format("{}:{}:{}", "input_file", token.pos.line, token.pos.column),
        token.lexeme);
  }
}

void Lexer::next_char() {
  if (current_pos >= size) {
    return;
  }

  else {
    current_char = source[current_pos++];
    current_column++;
  }
  if (current_char == '\n') {
    current_line++;
    current_column = 1;
  }

  if (current_pos >= size) {
    at_eof = true;
  }
}

std::optional<Token> Lexer::next_token() {
  if (isspace(current_char))
    skip_space();

  if (at_eof)
    return Token(TokenKind::END_OF_FILE, "EOF",
                 Position{current_line, current_column});

  // Taking a snapshot of the start of the current lexeme
  start_line = current_line;
  start_column = current_column - 1;

  if (current_char == '#') {
    skip_line();
    return next_token();
  }

  if (isalpha(current_char) || current_char == '_') {
    return read_ident_or_keyword();
  }

  if (isdigit(current_char)) {
    return read_num();
  }

  if (current_char == '\'') {
    return read_char();
  }

  if (current_char == '\"') {
    return read_string();
  }

  else
    return read_symbol();
}

std::optional<Token> Lexer::read_char() {
  next_char();
  char value;
  if (current_char == '\\') {
    // Escape Character
    next_char();
    switch (current_char) {
    case 'n':
      value = '\n';
      break;
    case 't':
      value = '\t';
      break;
    case '\\':
      value = '\\';
      break;
    case '\'':
      value = '\'';
      break;
    case '\"':
      value = '\"';
      break;
    default:
      // Illegal Escape Character, in g++ this is a warning instead of error
      // though
      diag.error(
          Position{start_line, start_column},
          fmt::format("unknown escape sequence: \'\\{}\'", current_char));
    }
  }

  // '' not legal:
  // g++ says the situation above is empty character constant
  else if (current_char == '\"') {
    diag.error(Position{start_line, start_column}, "empty character constant");
  }

  else {
    value = current_char;
  }

  next_char();

  if (current_char == '\'') {
    next_char();
    std::string lexeme = fmt::format("'{}'", value);
    return Token(TokenKind::CHAR_LITERAL, std::move(lexeme),
                 Position{start_line, start_column}, value);
  }

  // Default case: the char did not end with ', should throw error
  // g++ says: error: missing terminating ' character
  return Token{TokenKind::BAD, "missing terminating \' character",
               Position{start_line, start_column}};
}

std::optional<Token> Lexer::read_ident_or_keyword() {
  std::string lexeme;
  while (
      (isalpha(current_char) || isdigit(current_char) || current_char == '_') &&
      !at_eof) {
    lexeme += current_char;
    next_char();
  }
  if (keyword_map.contains(lexeme)) {
    return Token{keyword_map.at(lexeme), std::move(lexeme),
                 Position{start_line, start_column}};
  } else {
    return Token{TokenKind::IDENTIFIER, std::string(lexeme),
                 Position{start_line, start_column}, std::move(lexeme)};
  }
}

std::optional<Token> Lexer::read_num() {
  // TODO: The current string(char) to int conversion is poor.
  // Could have some improvment to support more types of nums.

  // Hexadecimal numbers
  if (current_char == '0') {
    next_char();
    if (current_char == 'x' || current_char == 'X') {
      next_char();
      int k = 0;
      std::string lexeme;
      while (isxdigit(current_char)) {
        lexeme += current_char;
        k = k * 16;
        if (isdigit(current_char)) {
          k = k + current_char - '0';
        } else {
          switch (current_char) {
          case 'a' | 'A':
            k += 10;
          case 'b' | 'B':
            k += 11;
          case 'c' | 'C':
            k += 12;
          case 'd' | 'D':
            k += 13;
          case 'e' | 'E':
            k += 14;
          case 'f' | 'F':
            k += 15;
          }
        }
        next_char();
      }
      return Token{TokenKind::INT_LITERAL, std::move(lexeme),
                   Position{start_line, start_column}, k};
    }
  }

  // Regular numbers (decimal point not supported currently)
  int k = 0;
  std::string lexeme;
  while (isdigit(current_char)) {
    k = k * 10 + current_char - '0';
    lexeme += current_char;
    next_char();
  }
  return Token{TokenKind::INT_LITERAL, std::move(lexeme),
               Position{start_line, start_column}, k};
}

std::optional<Token> Lexer::read_string() {
  std::string lexeme;
  next_char();
  while (current_char != '\"') {
    if (current_char == '\\') {
      // Escape Character
      next_char();
      switch (current_char) {
      case 'n':
        lexeme += '\n';
        break;
      case 't':
        lexeme += '\t';
        break;
      case '\\':
        lexeme += '\\';
        break;
      case '\'':
        lexeme += '\'';
        break;
      case '\"':
        lexeme += '\"';
        break;
      default:
        // Illegal Escape Character, in g++ this is a warning instead of error
        // though
        diag.error(
            Position{start_line, start_column},
            fmt::format("unknown escape sequence: \'\\{}\'", current_char));
      }
    }

    else if (current_char == '\n') {
      // According to g++, an newline in double quote is illegal
      diag.error(Position{start_line, start_column},
                 "missing terminating \" character");
      return Token{TokenKind::BAD, "missing terminating \" character",
                   Position{start_line, start_column}};
    }

    else {
      lexeme += current_char;
    }

    next_char();
  }
  next_char(); // Consume the terminating quote
  std::string lexeme_with_quote = fmt::format("\"{}\"", lexeme);
  return Token(TokenKind::STRING_LITERAL, std::move(lexeme_with_quote),
               Position(start_line, start_column), std::move(lexeme));
}

std::optional<Token> Lexer::read_symbol() {
  switch (current_char) {
  case '~':
    next_char();
    return Token(TokenKind::BITWISE_NOT, "~",
                 Position(start_line, start_column));
  case '+':
    next_char();
    return Token(TokenKind::PLUS, "+", Position(start_line, start_column));
  case '-':
    next_char();
    return Token(TokenKind::MINUS, "-", Position(start_line, start_column));
  case '*':
    next_char();
    return Token(TokenKind::MULTIPLY, "*", Position(start_line, start_column));
  case '^':
    next_char();
    return Token(TokenKind::EXPONENTIAL, "^",
                 Position(start_line, start_column));
  case '%':
    next_char();
    return Token(TokenKind::MODULUS, "%", Position(start_line, start_column));
  case '(':
    next_char();
    return Token(TokenKind::LPAREN, "(", Position(start_line, start_column));
  case ')':
    next_char();
    return Token(TokenKind::RPAREN, ")", Position(start_line, start_column));
  case '[':
    next_char();
    return Token(TokenKind::LBRACKET, "[", Position(start_line, start_column));
  case ']':
    next_char();
    return Token(TokenKind::RBRACKET, "]", Position(start_line, start_column));
  case '{':
    next_char();
    return Token(TokenKind::LBRACE, "{", Position(start_line, start_column));
  case '}':
    next_char();
    return Token(TokenKind::RBRACE, "}", Position(start_line, start_column));
  case ';':
    next_char();
    return Token(TokenKind::SEMICOLON, ";", Position(start_line, start_column));
  case ',':
    next_char();
    return Token(TokenKind::COMMA, ",", Position(start_line, start_column));
  case '.':
    next_char();
    return Token(TokenKind::PERIOD, ".", Position(start_line, start_column));

  case '!': {
    next_char();
    switch (current_char) {
    case '=':
      next_char();
      return Token(TokenKind::NOT_EQUAL,
                   "!=", Position(start_line, start_column));
    default:
      return Token(TokenKind::NOT, "!", Position(start_line, start_column));
    }
  }
  // Ignore pre-processing, need to be implemented for real c++ code
  case '#': {
    skip_line();
    return next_token();
  }
  // Comments
  case '/': {
    next_char();
    if (current_char == '/') {
      skip_line();
      return next_token();
    }

    else if (current_char == '*') {
      while (current_pos < size) {
        if (current_char == '*') {
          next_char();
          if (current_char == '/')
            break;
        } else {
          next_char();
        }
      }
      if (current_pos >= size) {
        diag.error(Position(start_line, start_column), "unterminated comment");
        return Token(TokenKind::BAD, "unterminated comment",
                     Position(start_line, start_column));
      }
      next_char();
      return next_token();
    }

    else {
      next_char();
      return Token(TokenKind::DIVIDE, "/", Position(start_line, start_column));
    }
  }

  case '<': {
    next_char();
    switch (current_char) {
    case '<':
      next_char();
      return Token(TokenKind::SHIFT_LEFT, "<<",
                   Position(start_line, start_column));
    case '=':
      next_char();
      return Token(TokenKind::LESS_EQUAL,
                   "<=", Position(start_line, start_column));
    default:
      return Token(TokenKind::LESS_THAN, "<",
                   Position(start_line, start_column));
    }
  }

  case '>': {
    next_char();
    switch (current_char) {
    case '>':
      next_char();
      return Token(TokenKind::SHIFT_RIGHT, ">>",
                   Position(start_line, start_column));
    case '=':
      next_char();
      return Token(TokenKind::LARGER_EQUAL,
                   ">=", Position(start_line, start_column));
    default:
      return Token(TokenKind::LARGER_THAN, ">",
                   Position(start_line, start_column));
    }
  }

  case '=': {
    next_char();
    switch (current_char) {
    case '=':
      next_char();
      return Token(TokenKind::EQUAL, "==", Position(start_line, start_column));
    default:
      return Token(TokenKind::ASSIGN, "=", Position(start_line, start_column));
    }
  }

  case '|': {
    next_char();
    switch (current_char) {
    case '|':
      next_char();
      return Token(TokenKind::LOGICAL_OR, "||",
                   Position(start_line, start_column));
    default:
      return Token(TokenKind::BITWISE_OR, "|",
                   Position(start_line, start_column));
    }
  }

  case '&': {
    next_char();
    switch (current_char) {
    case '&':
      next_char();
      return Token(TokenKind::LOGICAL_AND, "&&",
                   Position(start_line, start_column));
    default:
      return Token(TokenKind::BITWISE_AND, "&",
                   Position(start_line, start_column));
    }
  }

  default: {
    diag.error(Position(start_line, start_column), "undefined symbol");
    next_char();
    return Token(TokenKind::BAD, "undefined symbol",
                 Position(start_line, start_column));
  }
  }
}

void Lexer::skip_line() {
  while (current_pos < size && current_char != '\n') {
    next_char();
  }
}

void Lexer::skip_space() {
  while (current_pos < size && isspace(current_char)) {
    next_char();
  }
}
