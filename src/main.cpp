#include "lexer.h"
#include "parser.h"
#include "evaluator.h"
#include <iostream>
#include <fstream>
#include <filesystem>

int main(int argc, char* argv[]){
    if(argc < 2){
        std::cerr << "Usage : abax <fichier.abax>" << std::endl;
        return 1;
    }

    std::ifstream file(argv[1]);
    if(!file.is_open()){
        std::cerr << "Impossible d'ouvrir : " << argv[1] << std::endl;
        return 1;
    }
    
    std::string source((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    std::string out_path = std::filesystem::path(argv[1]).replace_extension(".out").string();
    std::ofstream out(out_path);

    // Lexing
    std::vector<Token> tokens;
    try {
        Lexer lexer(source);
        tokens = lexer.tokenize();
    } catch(const std::exception& e){
        std::cerr << "[Erreur Lexer] " << e.what() << std::endl;
        return 1;
    }

    // Parsing et évaluation expression par expression
    Evaluator evaluator;
    std::vector<Token> segment;
    int expr_index = 1;

    for(auto& token : tokens){
        if(token.type == TokenType::SEMICOLON || token.type == TokenType::END){
            if(segment.empty()){
                if(token.type == TokenType::END) break;
                continue;
            }

            segment.emplace_back(TokenType::END);

            // Reconstruit l'expression en texte pour l'affichage
            std::string expr_text;
            for(auto& t : segment)
                if(t.type != TokenType::END)
                    expr_text += t.value.empty() ? std::string(1, token_char(t.type)) : t.value + " ";

            try {
                Parser parser(segment);
                auto ast = parser.parse();
                Value result = evaluator.evaluate(ast.get());

                std::string result_str;
                if(std::holds_alternative<long long>(result))
                    result_str = std::to_string(std::get<long long>(result));
                else {
                    double d = std::get<double>(result);
                    if(d == std::floor(d) && std::abs(d) < 1e15)
                        result_str = std::to_string((long long)d);
                    else
                        result_str = std::to_string(d);
                }

                std::string line = result_str + " ; ";
                out << line << std::endl;
            } catch(const std::exception& e){
                out << e.what() << std::endl;
            }

            segment.clear();
            expr_index++;

            if(token.type == TokenType::END) break;
        } else {
            segment.push_back(token);
        }
    }

    std::cout << out_path << std::endl;
    return 0;
}