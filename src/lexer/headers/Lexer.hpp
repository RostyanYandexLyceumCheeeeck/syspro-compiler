#pragma once

#include <vector>
#include <string_view>

#include "Token.hpp"


class Lexer {
public:
    Lexer() = default;
    std::vector<Token> parse(std::string_view src) const;
};