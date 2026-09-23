#include <fstream>
#include <sstream>

#include "headers/Lexer.hpp"


int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <input_file.spl>\n";
        return -1;
    }

    std::ifstream file(argv[1]);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << argv[1] << "\n";
        return -2;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string source = buffer.str();

    Lexer lexer;
    auto tokens = lexer.parse(source);

    int codeReturn = 0;
    nlohmann::ordered_json j = nlohmann::ordered_json::array();
    
    for (const auto& tok : tokens) {
        auto tokType = tok.getType();
        if (tokType == SPACE || tokType == SINGLE_COMM || tokType == MULTI_COMM) { continue; }
        else if (tokType == ERROR) { codeReturn++; }
        
        j.push_back(tok.toJson());
    }

    std::cout << j.dump(2) << std::endl;
    return codeReturn;
}