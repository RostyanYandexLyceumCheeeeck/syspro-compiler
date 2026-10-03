#pragma once

#include <ctre.hpp>

#include "Token.hpp"


template <size_t N>
struct FixedString {
    char buf[N];
    constexpr FixedString(const char (&s)[N]) { std::copy_n(s, N, buf); }

    constexpr explicit operator bool() const { return N > 1 && buf[0] != '\0'; }
    constexpr operator std::string_view() const { return {buf, N > 0 ? N - 1 : 0}; }
};


// Interface TokenParse
class ITokenParse {
public:
    virtual ~ITokenParse() = default;
    virtual std::optional<Token> getToken(const std::string& src, size_t& offset, size_t& line, size_t& column) = 0;
};


template <FixedString Pattern, FixedString Name, TokenType T, FixedString ValueError = "">
class TokenParse: public ITokenParse {

public:
    std::optional<Token> getToken(const std::string& src, size_t& offset, size_t& line, size_t& column) override {
        if (offset >= src.size()) { return std::nullopt; }
        
        auto match = ctre::starts_with<Pattern.buf>(src.begin() + offset, src.end());
        if (!match) { return std::nullopt; }

        std::string_view sv = match.to_view();
        Token result{T, 
                     ValueError ? std::string(ValueError) : std::string(sv), 
                     std::string(Name), 
                     line, column};

        offset += sv.length();
        column += sv.length();
        
        auto last_n = sv.rfind('\n');
        if (last_n != std::string_view::npos) {
            line += std::count(sv.begin(), sv.end(), '\n');
            column = sv.length() - last_n;
        }
        return result;
    }
};
