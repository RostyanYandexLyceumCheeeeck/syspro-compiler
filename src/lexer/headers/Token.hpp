#pragma once

#include <string>
#include <iostream>
#include <string_view>

#include <nlohmann/json.hpp>

#define TOKEN_LIST(X)                       \
    /* keywords */                          \
    X(RETURN) X(VAL)      X(VAR)            \
    X(IF)     X(ELSE)     X(WHILE)          \
    X(BREAK)  X(CONTINUE) X(TRUE)           \
    X(FALSE)  X(DEF)      X(EXTERN)         \
    X(INT8)   X(INT16)    X(INT32)          \
    X(INT64)  X(BOOL)     X(STRING)         \
    X(CAST)   X(STRUCT)                     \
                                            \
    /* space, error, string and comments */ \
    X(SPACE) X(ERROR) X(STR)                \
    X(SINGLE_COMM) X(MULTI_COMM)            \
                                            \
    /* number and ident */                  \
    X(INT) X(IDENT)                         \
                                            \
    /* brackets */                          \
    X(LPAREN) X(RPAREN)                     \
    X(LBRACE) X(RBRACE)                     \
    X(LBRACKET) X(RBRACKET)                 \
                                            \
    /* operations */                        \
    X(AND)    X(OR)   X(LE)                 \
    X(GE)     X(NE)   X(EQ)                 \
    X(ASSIGN) X(LT)   X(GT)                 \
    X(NOT)    X(PLUS) X(MINUS)              \
    X(MULT)   X(DIV)                        \
                                            \
    /* dot, colon, comma and semicolon */   \
    X(SEMI) X(DOT) X(COLON) X(COMMA)        \
                                            \
    /* EOF */                               \
    X(END)                                  \


enum class TokenType {
    #define AS_ENUM(name) name,
    TOKEN_LIST(AS_ENUM)
    #undef AS_ENUM
};


constexpr std::string_view tokenTypeToString(TokenType type) {
    if (type == TokenType::END) {
        return "EOF";
    }

    switch (type) {
        #define AS_STRING(name) case TokenType::name: return #name;
        TOKEN_LIST(AS_STRING)
        #undef AS_STRING
    }
    return "UNKNOWN";
}


inline TokenType tokenTypeFromString(std::string_view name) {
    if (name == "EOF") {
        return TokenType::END;
    }
    #define AS_STRING(t) if (name == #t) return TokenType::t;
    TOKEN_LIST(AS_STRING)
    #undef AS_STRING
    return TokenType::ERROR;
}


class Token {
    TokenType ttype_;
    std::string value_;
    size_t line_;
    size_t column_;

public:
    Token() = default;
    Token(TokenType t, std::string val, size_t l, size_t col): ttype_(t), value_(std::move(val)), 
                                                               line_(l), column_(col) {}

    TokenType getType() const { return ttype_; }

    std::string_view getName() const { return tokenTypeToString(ttype_); }
    const std::string& getValue() const { return value_; }
    size_t getLine() const { return line_; }
    size_t getColumn() const { return column_; }

    nlohmann::ordered_json toJson() const {
        nlohmann::ordered_json res;
        
        res["kind"] = getName();
        res["value"] = value_;
        res["line"] = line_;
        res["column"] = column_;

        return res;
    }

    friend 
    std::ostream& operator<<(std::ostream& os, const Token& t) {
        os << t.toJson().dump();
        return os;
    }
};

