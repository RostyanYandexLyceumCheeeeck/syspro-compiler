#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>

#include <headers/CliArgs.hpp>
#include <headers/Parser.hpp>


int main(int argc, char* argv[]) {
    auto temp = parseCliArgs(argc, argv, "-a", "ast_out", "tokens.json");
    if (!temp) { return -200; }
    CliArgs args = temp.value();

    std::ifstream inFile(args.inputPath);
    if (!inFile.is_open()) {
        std::cerr << "Failed to open input file: " << args.inputPath << "\n";
        return -201;
    }
    
    Parser parser(Parser::loadTokensFromJson(args.inputPath), args.grammar);
    auto ast = parser.parseProgram();

    if (ast) {
        if (!args.outputPath.empty()) {
            std::ofstream outFile(args.outputPath);
        
            if (!outFile.is_open()) {
                std::cerr << "Failed to open output file: " << args.outputPath << "\n";
                return -202;
            }
            outFile << ast->toJson().dump(2) << '\n';
        } 
        else {
            std::cout << ast->toJson().dump(2) << std::endl;
        }
    }

    if (parser.hasError()) {
        return -203;
    }

    return 0;
}