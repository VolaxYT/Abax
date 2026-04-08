#pragma once
#include "parser.h"
#include <cmath>
#include <stdexcept>

using Value = std::variant<long long, double>;

class Evaluator {
public:
    Value evaluate(const Node* node){
    if(auto* n = dynamic_cast<const NumberNode*>(node)){
        if(n->is_float) return n->value_f;
        return n->value_i;
    }

    if(auto* n = dynamic_cast<const BinaryNode*>(node)){
        Value left  = evaluate(n->left.get());
        Value right = evaluate(n->right.get());

        // Si l'un des deux est float, tout passe en float
        bool any_float = std::holds_alternative<double>(left)
                      || std::holds_alternative<double>(right);

        auto to_ll = [](Value v) -> long long {
            return std::holds_alternative<long long>(v)
                ? std::get<long long>(v)
                : (long long)std::get<double>(v);
        };
        auto to_d = [](Value v) -> double {
            return std::holds_alternative<double>(v)
                ? std::get<double>(v)
                : (double)std::get<long long>(v);
        };

        switch(n->op){
            case '+': return any_float ? Value(to_d(left) + to_d(right))
                                       : Value(to_ll(left) + to_ll(right));
            case '-': return any_float ? Value(to_d(left) - to_d(right))
                                       : Value(to_ll(left) - to_ll(right));
            case '*': return any_float ? Value(to_d(left) * to_d(right))
                                       : Value(to_ll(left) * to_ll(right));
            case '/':
                if(to_d(right) == 0.0) throw std::runtime_error("Division par zéro");
                // La division retourne toujours un float
                return Value(to_d(left) / to_d(right));
            case '%':
                if(to_ll(right) == 0) throw std::runtime_error("Modulo par zéro");
                return any_float ? Value(std::fmod(to_d(left), to_d(right)))
                                 : Value(to_ll(left) % to_ll(right));
            case '^': return Value(std::pow(to_d(left), to_d(right)));
            default:  throw std::runtime_error(std::string("Opérateur inconnu : ") + n->op);
        }
    }

    if(auto* n = dynamic_cast<const UnaryNode*>(node)){
        Value val = evaluate(n->operand.get());
        if(n->op == '-'){
            if(std::holds_alternative<long long>(val))
                return Value(-std::get<long long>(val));
            return Value(-std::get<double>(val));
        }
        throw std::runtime_error("Opérateur unaire inconnu");
    }

    if(auto* n = dynamic_cast<const FunctionNode*>(node)){
        // Les fonctions travaillent toujours en double
        Value arg_val = evaluate(n->argument.get());
        double arg = std::holds_alternative<double>(arg_val)
                   ? std::get<double>(arg_val)
                   : (double)std::get<long long>(arg_val);

        if(n->name == "sqrt"){
            if(arg < 0) throw std::runtime_error("sqrt d'un nombre négatif");
            return Value(std::sqrt(arg));
        }
        if(n->name == "sin") return Value(std::sin(arg));
        if(n->name == "cos") return Value(std::cos(arg));
        if(n->name == "tan") return Value(std::tan(arg));
        if(n->name == "log") return Value(std::log(arg));
        if(n->name == "log2") return Value(std::log2(arg));
        if(n->name == "abs") return Value(std::abs(arg));
        if(n->name == "floor") return Value(std::floor(arg));
        if(n->name == "ceil") return Value(std::ceil(arg));

        throw std::runtime_error("Fonction inconnue : " + n->name);
    }

    throw std::runtime_error("Noeud AST inconnu");
}
};