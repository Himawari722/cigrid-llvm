#pragma once

#include <string>
#include <variant>

enum class Kind {
    // Operators
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
    // Separators
    LPAREN,
    RPAREN,
    LBRACKET,
    RBRACKET,
    LBRACE,
    RBRACE,
    SEMICOLON,
    COMMA,
    PERIOD,
    // Keywords
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
    // Miscellaneous
    BAD
};


struct IntToken { int value; };

struct CharToken { char value; };

struct StringToken { std::string value; };

struct IdentToken { std::string value; };

struct EndOfFile {};

using TokenKind = std::variant<Kind, IntToken, CharToken, StringToken, IdentToken, EndOfFile>;

struct Token {
    TokenKind kind;
    std::string lexeme;
    int line;
    int column;
};