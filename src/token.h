#pragma once
#include <string>
#include <variant>

enum class TokenType {
    NUMBER,

    PLUS,
    MINUS,
    STAR,
    SLASH,
    CARET,
    PERCENT,

    LPAREN,
    RPAREN,

    FUNCTION,

    SEMICOLON,
    END,
};

// Retourne le caractère associé à un token opérateur
inline char token_char(TokenType t){
    switch(t){
        case TokenType::PLUS: return '+';
        case TokenType::MINUS: return '-';
        case TokenType::STAR: return '*';
        case TokenType::SLASH: return '/';
        case TokenType::CARET: return '^';
        case TokenType::PERCENT: return '%';
        case TokenType::LPAREN: return '(';
        case TokenType::RPAREN: return ')';
        default: return '?';
    }
}

struct Token {
    TokenType   type;
    std::string value;

    Token(TokenType t, std::string v = "") : type(t), value(std::move(v)){}
};