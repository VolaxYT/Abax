#pragma once
#include "token.h"
#include <vector>
#include <stdexcept>
#include <cctype>
#include <unordered_set>

class Lexer {
public:
    explicit Lexer(const std::string& source) : src(source), pos(0) {}

    std::vector<Token> tokenize(){
        std::vector<Token> tokens;

        while(pos < src.size()){
            skip_whitespace();
            if(pos >= src.size()) break;

            char c = src[pos];

            if(std::isdigit(c) || c == '.'){
                tokens.push_back(read_number());
                continue;
            }
            
            if(std::isalpha(c)){ 
                tokens.push_back(read_function());
                continue;
            }

            switch(c){
                case '+': tokens.emplace_back(TokenType::PLUS); break;
                case '-': tokens.emplace_back(TokenType::MINUS); break;
                case '*': tokens.emplace_back(TokenType::STAR); break;
                case '/': tokens.emplace_back(TokenType::SLASH); break;
                case '^': tokens.emplace_back(TokenType::CARET); break;
                case '%': tokens.emplace_back(TokenType::PERCENT); break;
                case '(': tokens.emplace_back(TokenType::LPAREN); break;
                case ')': tokens.emplace_back(TokenType::RPAREN); break;
                case ';': tokens.emplace_back(TokenType::SEMICOLON); break;
                default:
                    throw std::runtime_error(
                        std::string("Caractère inconnu : '") + c + "'"
                    );
            }
            pos++;
        }

        tokens.emplace_back(TokenType::END);
        return tokens;
    }

private:
    std::string src;
    size_t pos;

    void skip_whitespace(){
        while(pos < src.size() && std::isspace(src[pos]))
            pos++;
    }

    Token read_number(){
        size_t start = pos;
        bool has_dot = false;

        while(pos < src.size() && (std::isdigit(src[pos]) || src[pos] == '.')){
            if(src[pos] == '.'){
                if(has_dot)
                    throw std::runtime_error("Nombre malformé : plusieurs points décimaux");
                has_dot = true;
            }
            pos++;
        }
        return Token(TokenType::NUMBER, src.substr(start, pos - start) + (has_dot ? "f" : "i"));
    }

    Token read_function(){
        static const std::unordered_set<std::string> known = {
            "sqrt", "sin", "cos", "tan", "log", "log2", "abs", "floor", "ceil"
        };

        size_t start = pos;
        while(pos < src.size() && std::isalpha(src[pos]))
            pos++;

        std::string name = src.substr(start, pos - start);
        if(known.find(name) == known.end())
            throw std::runtime_error("Fonction inconnue : '" + name + "'");

        return Token(TokenType::FUNCTION, name);
    }
};