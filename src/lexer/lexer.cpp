#include "lexer/lexer.hpp"
#include "fmt/core.h"

#include <cctype>
#include <iostream>
#include <optional>

Lexer::Lexer(std::string_view input, Diagnostics &diag) : source(input), diag(diag) { next_char(); }

std::vector<Token> Lexer::gen_token() {
  // next_token returns optional<token>, so can be place inside while
  while (auto token_opt = next_token()) {
    // dereference to get the Token inside optional object
    auto &token = *token_opt;
    token_list.push_back(*token_opt);
    // token_list.push_back(std::move(*token_opt));
    if (std::holds_alternative<EndOfFile>(token.kind))
      break;
  }
  return token_list;
}

void Lexer::print_token_list() {
  for (const auto &token : token_list) {
    // Only suppor single file here;
    fmt::print("{:<20}{}\n", fmt::format("{}:{}:{}", "input_file", token.line, token.column), token.lexeme);
  }
}

void Lexer::next_char() {
  if (current_pos >= size)
    at_eof = true;
  else {
    current_char = source[current_pos++];
    current_column++;
  }
  if (current_char == '\n') {
    current_line++;
    current_column = 1;
  }
}

std::optional<Token> Lexer::next_token() {
  if (isspace(current_char))
    skip_space();

  if (at_eof)
    return Token(EndOfFile(), "EOF", current_line, current_column);

  // Taking a snapshot of the start of the current lexeme
  start_line = current_line;
  start_column = current_column-1;
  

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
      // Illegal Escape Character, in g++ this is a warning instead of error though
      diag.error(start_line, start_column, fmt::format("unknown escape sequence: \'\\{}\'", current_char));
    }
  }

  // '' not legal:
  // g++ says the situation above is empty character constant
  else if (current_char == '\"') {
    diag.error(start_line, start_column, "empty character constant");
  }

  else {
    value = current_char;
  }

  next_char();

  if (current_char == '\'') {
    next_char();
    std::string lexeme = {'\'', value, '\''};
    return Token(CharToken(value), std::string(lexeme), start_line,
                 start_column);
  }

  // Default case: the char did not end with ', should throw error
  // g++ says: error: missing terminating ' character
  return Token(Kind::BAD, "missing terminating \' character", current_line,
               current_column);
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
    return Token(keyword_map.at(lexeme), std::string(lexeme), start_line,
                 start_column);
  } else
    return Token(IdentToken(lexeme), std::string(lexeme), start_line,
                 start_column);
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
      return Token(IntToken(k), std::string(lexeme), start_line, start_column);
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
  return Token(IntToken(k), std::string(lexeme), start_line, start_column);
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
        // Illegal Escape Character, in g++ this is a warning instead of error though
        diag.error(start_line, start_column, fmt::format("unknown escape sequence: \'\\{}\'", current_char));
      }
    }

    else if (current_char == '\n') {
      // According to g++, an newline after double quote is illegal
      diag.error(start_line, start_column, "missing terminating \" character");
      return Token(Kind::BAD, "missing terminating \" character", current_line,
                   current_column);
    }

    else {
      lexeme += current_char;
    }

    next_char();
  }
  return Token(StringToken(lexeme), lexeme, start_line, start_column);
}

std::optional<Token> Lexer::read_symbol() {
  switch (current_char) {
  case '~':
    next_char();
    return Token(Kind::BITWISE_NOT, "~", start_line, start_column);
  case '+':
    next_char();
    return Token(Kind::PLUS, "+", start_line, start_column);
  case '-':
    next_char();
    return Token(Kind::MINUS, "-", start_line, start_column);
  case '*':
    next_char();
    return Token(Kind::MULTIPLY, "*", start_line, start_column);
  case '^':
    next_char();
    return Token(Kind::EXPONENTIAL, "^", start_line, start_column);
  case '%':
    next_char();
    return Token(Kind::MODULUS, "%", start_line, start_column);
  case '(':
    next_char();
    return Token(Kind::LPAREN, "(", start_line, start_column);
  case ')':
    next_char();
    return Token(Kind::RPAREN, ")", start_line, start_column);
  case '[':
    next_char();
    return Token(Kind::LBRACKET, "[", start_line, start_column);
  case ']':
    next_char();
    return Token(Kind::RBRACKET, "]", start_line, start_column);
  case '{':
    next_char();
    return Token(Kind::LBRACE, "{", start_line, start_column);
  case '}':
    next_char();
    return Token(Kind::RBRACE, "}", start_line, start_column);
  case ';':
    next_char();
    return Token(Kind::SEMICOLON, ";", start_line, start_column);
  case ',':
    next_char();
    return Token(Kind::COMMA, ",", start_line, start_column);
  case '.':
    next_char();
    return Token(Kind::PERIOD, ".", start_line, start_column);

  case '!': {
    next_char();
    switch (current_char) {
    case '=':
      next_char();
      return Token(Kind::NOT_EQUAL, "!=", start_line, start_column);
    default:
      return Token(Kind::NOT, "!", start_line, start_column);
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
        diag.error(start_line, start_column, "unterminated comment");
        return Token(Kind::BAD, "unterminated comment", start_line,
                     start_column);
      }
      next_char();
      return next_token();
    }

    else {
      next_char();
      return Token(Kind::DIVIDE, "/", start_line, start_column);
    }
  }

  case '<': {
    next_char();
    switch (current_char) {
    case '<':
      next_char();
      return Token(Kind::SHIFT_LEFT, "<<", start_line, start_column);
    case '=':
      next_char();
      return Token(Kind::LESS_EQUAL, "<=", start_line, start_column);
    default:
      return Token(Kind::LESS_THAN, "<", start_line, start_column);
    }
  }

  case '>': {
    next_char();
    switch (current_char) {
    case '>':
      next_char();
      return Token(Kind::SHIFT_RIGHT, ">>", start_line, start_column);
    case '=':
      next_char();
      return Token(Kind::LARGER_EQUAL, ">=", start_line, start_column);
    default:
      return Token(Kind::LARGER_THAN, ">", start_line, start_column);
    }
  }

  case '=': {
    next_char();
    switch (current_char) {
    case '=':
      next_char();
      return Token(Kind::EQUAL, "==", start_line, start_column);
    default:
      return Token(Kind::ASSIGN, "=", start_line, start_column);
    }
  }

  case '|': {
    next_char();
    switch (current_char) {
    case '|':
      next_char();
      return Token(Kind::LOGICAL_OR, "||", start_line, start_column);
    default:
      return Token(Kind::BITWISE_OR, "|", start_line, start_column);
    }
  }

  case '&': {
    next_char();
    switch (current_char) {
    case '&':
      next_char();
      return Token(Kind::LOGICAL_AND, "&&", start_line, start_column);
    default:
      return Token(Kind::BITWISE_AND, "&", start_line, start_column);
    }
  }

  default: {
    diag.error(start_line, start_column, "undefined symbol");
    next_char();
    return Token(Kind::BAD, "undefined symbol", start_line, start_column);
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
