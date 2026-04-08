#pragma once
#include "token.h"
#include <vector>
#include <memory>
#include <stdexcept>

// Noeuds de l'AST

struct Node {
    virtual ~Node() = default;
};

// Nombre littéral
struct NumberNode : Node {
    double value_f;
    long long value_i;
    bool is_float;
    explicit NumberNode(double v, bool f) : value_f(v), value_i((long long)v), is_float(f) {}
};

// Opération binaire
struct BinaryNode : Node {
    char op;
    std::unique_ptr<Node> left;
    std::unique_ptr<Node> right;

    BinaryNode(char o, std::unique_ptr<Node> l, std::unique_ptr<Node> r) : op(o), left(std::move(l)), right(std::move(r)) {}
};

// Opération unaire
struct UnaryNode : Node {
    char op;
    std::unique_ptr<Node> operand;

    UnaryNode(char o, std::unique_ptr<Node> n) : op(o), operand(std::move(n)) {}
};

// Appel de fonction
struct FunctionNode : Node {
    std::string name;
    std::unique_ptr<Node> argument;

    FunctionNode(std::string n, std::unique_ptr<Node> arg) : name(std::move(n)), argument(std::move(arg)) {}
};

// Parser

class Parser {
public:
    explicit Parser(std::vector<Token> tokens) : tokens(std::move(tokens)), pos(0) {}

    // Parse une expression complète et retourne la racine de l'AST
    std::unique_ptr<Node> parse(){
        auto node = parse_expression();
        return node;
    }

private:
    std::vector<Token> tokens;
    size_t pos;

    // Regarde le token courant sans avancer
    const Token& peek() const { return tokens[pos]; }

    // Consomme le token courant et avance
    const Token& advance(){ return tokens[pos++]; }

    // Vérifie si le token courant est d'un certain type
    bool check(TokenType t) const { return peek().type == t; }

    // Consomme le token courant si il correspond
    bool match(TokenType t){
        if(check(t)){ advance(); return true; }
        return false;
    }

    // Niveau 1 — addition et soustraction (priorité la plus basse)
    std::unique_ptr<Node> parse_expression(){
        auto left = parse_term();

        while(check(TokenType::PLUS) || check(TokenType::MINUS)){
            char op = advance().value.empty() ? (tokens[pos-1].type == TokenType::PLUS ? '+' : '-') : advance().value[0];
            op = (tokens[pos-1].type == TokenType::PLUS) ? '+' : '-';
            auto right = parse_term();
            left = std::make_unique<BinaryNode>(op, std::move(left), std::move(right));
        }
        return left;
    }

    // Niveau 2 — multiplication, division, modulo
    std::unique_ptr<Node> parse_term(){
        auto left = parse_power();

        while(check(TokenType::STAR) || check(TokenType::SLASH) || check(TokenType::PERCENT)){
            char op;
            if(check(TokenType::STAR)){
                op = '*'; advance();
            } else if(check(TokenType::SLASH)){
                op = '/'; advance();
            } else{
                op = '%'; advance();
            }
            auto right = parse_power();
            left = std::make_unique<BinaryNode>(op, std::move(left), std::move(right));
        }
        return left;
    }

    // Niveau 3 — puissance
    std::unique_ptr<Node> parse_power(){
        auto left = parse_unary();

        if(check(TokenType::CARET)){
            advance();
            // Associativité droite — on rappelle parse_power récursivement
            auto right = parse_power();
            return std::make_unique<BinaryNode>('^', std::move(left), std::move(right));
        }
        return left;
    }

    // Niveau 4 — unaire : négation
    std::unique_ptr<Node> parse_unary(){
        if(check(TokenType::MINUS)){
            advance();
            auto operand = parse_unary();
            return std::make_unique<UnaryNode>('-', std::move(operand));
        }
        return parse_primary();
    }

    // Niveau 5 — primaire : nombre, fonction, parenthèses
    std::unique_ptr<Node> parse_primary(){

        // Nombre littéral
        if(check(TokenType::NUMBER)){
            std::string raw = advance().value;
            bool is_float = raw.back() == 'f';
            std::string num = raw.substr(0, raw.size() - 1);
            double val = std::stod(num);
            return std::make_unique<NumberNode>(val, is_float);
        }

        // Appel de fonction : nom '(' expression ')'
        if(check(TokenType::FUNCTION)){
            std::string name = advance().value;
            if(!match(TokenType::LPAREN))
                throw std::runtime_error("'(' attendu après " + name);
            auto arg = parse_expression();
            if(!match(TokenType::RPAREN))
                throw std::runtime_error("')' attendu après argument de " + name);
            return std::make_unique<FunctionNode>(name, std::move(arg));
        }

        // Expression entre parenthèses
        if(match(TokenType::LPAREN)){
            auto node = parse_expression();
            if(!match(TokenType::RPAREN))
                throw std::runtime_error("')' attendu");
            return node;
        }

        throw std::runtime_error("Token inattendu : '" + peek().value + "'");
    }
};