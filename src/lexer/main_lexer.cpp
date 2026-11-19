#include <fstream>
#include <iostream>
#include <sstream>

#include <headers/Lexer.hpp>
#include <headers/CliArgs.hpp>

#define DEBUG 0


int main(int argc, char* argv[]) {
    auto temp = parseCliArgs(argc, argv, "-t", "tokens_out", "input_file.spl");
    if (!temp) { return -100; }
    CliArgs args = temp.value();

    std::ifstream inFile(args.inputPath);
    if (!inFile.is_open()) {
        std::cerr << "Failed to open input file: " << args.inputPath << "\n";
        return -101;
    }

    std::stringstream buffer;
    buffer << inFile.rdbuf();
    std::string source = buffer.str();

    Lexer lexer;
    auto tokens = lexer.parse(source);

    int codeReturn = 0;
    nlohmann::ordered_json j = nlohmann::ordered_json::array();

    for (const auto& tok : tokens) {
        auto tokType = tok.getType();

        if (tokType == TokenType::ERROR) { codeReturn = -103; }
#if (!DEBUG)
        else if (tokType == TokenType::SPACE ||
                 tokType == TokenType::SINGLE_COMM ||
                 tokType == TokenType::MULTI_COMM) {
            continue;
        }
#endif

        j.push_back(tok.toJson());
    }
    
    if (!args.outputPath.empty()) {
        std::ofstream outFile(args.outputPath);
        if (!outFile.is_open()) {
            std::cerr << "Failed to open output file: " << args.outputPath << "\n";
            return -102;
        }
        outFile << j.dump(2) << '\n';
    } 
    else {
        std::cout << j.dump(2) << std::endl;
    }

    return codeReturn;
}