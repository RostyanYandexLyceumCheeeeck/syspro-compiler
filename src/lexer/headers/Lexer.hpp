#pragma once

#include <iostream>

#include "TokenParse.hpp"


class Lexer {
    std::vector<std::unique_ptr<ITokenParse>> parsers_;

    void initParsers();

public:
    Lexer() { initParsers(); }
    std::vector<Token> parse(const std::string& src) const; 
};

