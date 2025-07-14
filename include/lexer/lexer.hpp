#pragma once

#include <optional>
#include <string_view>
#include <vector>
#include <unordered_map>

#include "diagnostics/diagnostics.hpp"
#include "token.hpp"

class Lexer {
public:
    explicit Lexer(std::string_view source, Diagnostics &diag);
    std::vector<Token> gen_token();
    void print_token_list();
private:
    void next_char();
    std::optional<Token> next_token();
    std::optional<Token> read_char();
    std::optional<Token> read_ident_or_keyword();
    std::optional<Token> read_num();
    std::optional<Token> read_string();
    std::optional<Token> read_symbol();
    void skip_line();
    void skip_space();
private:
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
    Diagnostics& diag;

    static const inline std::unordered_map<std::string, TokenKind> keyword_map = {
        {"break",   Kind::BREAK},
        {"char",    Kind::CHAR},
        {"else",    Kind::ELSE},
        {"extern",  Kind::EXTERN},
        {"for",     Kind::FOR},
        {"if",      Kind::IF},
        {"int",     Kind::INT},
        {"new",     Kind::NEW},
        {"return",  Kind::RETURN},
        {"struct",  Kind::STRUCT},
        {"void",    Kind::VOID},
        {"while",   Kind::WHILE}
    };
};