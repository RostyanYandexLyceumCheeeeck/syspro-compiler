#pragma once

#include <iostream>
#include <string>

#include <nlohmann/json.hpp>


enum TokenType {
    // keywords
    RETURN, VAL,      VAR,
    IF,     ELSE,     WHILE,
    BREAK,  CONTINUE, TRUE,
    FAlSE,  DEF,      EXTERN,
    INT8,   INT16,    INT32,
    INT64,  BOOL,     STRING,
    CAST,   STRUCT,
    
    // space, error, string and comments
    SPACE,  ERROR,    STR,
    SINGLE_COMM, MULTI_COMM,
    
    // number and ident
    INT, IDENT,

    // brackets
    LPAREN,   RPAREN,
    LBRACE,   RBRACE,
    LBRACKET, RBRACKET,

    // operations
    AND,    OR,   LE,
    GE,     NE,   EQ,
    ASSIGN, LT,   GT,
    NOT,    PLUS, MINUS,
    MULTI,  DIVI,

    // dot, colon, comma and semicolon
    SEMI, DOT, COLON, COMMA,
    
    // EOF
    END,
};


class Token {

    TokenType ttype_;
    std::string value_;
    std::string name_;
    size_t column_;
    size_t line_;

public:
    Token() = default;
    Token(TokenType t, std::string val, std::string name, size_t l, size_t col): ttype_(t), value_(std::move(val)), 
                                                                                     name_(name), 
                                                                                     line_(l), column_(col) {}
    
    TokenType getType() const { return ttype_; }

    nlohmann::ordered_json toJson() const {
        nlohmann::ordered_json res;
        res["kind"] = name_;
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

