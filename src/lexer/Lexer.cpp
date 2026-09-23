#include "headers/Lexer.hpp"


void Lexer::initParsers() {

    // keywords
    parsers_.push_back(std::make_unique<TokenParse<R"(return\b)",        "RETURN",      RETURN>>());
    parsers_.push_back(std::make_unique<TokenParse<R"(val\b)",           "VAL",         VAL>>());
    parsers_.push_back(std::make_unique<TokenParse<R"(var\b)",           "VAR",         VAR>>());
    parsers_.push_back(std::make_unique<TokenParse<R"(if\b)",            "IF",          IF>>());
    parsers_.push_back(std::make_unique<TokenParse<R"(else\b)",          "ELSE",        ELSE>>());
    parsers_.push_back(std::make_unique<TokenParse<R"(while\b)",         "WHILE",       WHILE>>());
    parsers_.push_back(std::make_unique<TokenParse<R"(break\b)",         "BREAK",       BREAK>>());
    parsers_.push_back(std::make_unique<TokenParse<R"(continue\b)",      "CONTINUE",    CONTINUE>>());
    parsers_.push_back(std::make_unique<TokenParse<R"(true\b)",          "TRUE",        TRUE>>());
    parsers_.push_back(std::make_unique<TokenParse<R"(false\b)",         "FALSE",       FAlSE>>());
    parsers_.push_back(std::make_unique<TokenParse<R"(def\b)",           "DEF",         DEF>>());
    parsers_.push_back(std::make_unique<TokenParse<R"(extern\b)",        "EXTERN",      EXTERN>>());
    parsers_.push_back(std::make_unique<TokenParse<R"(Int8\b)",          "INT8",        INT8>>());
    parsers_.push_back(std::make_unique<TokenParse<R"(Int16\b)",         "INT16",       INT16>>());
    parsers_.push_back(std::make_unique<TokenParse<R"(Int32\b)",         "INT32",       INT32>>());
    parsers_.push_back(std::make_unique<TokenParse<R"(Int64\b)",         "INT64",       INT64>>());
    parsers_.push_back(std::make_unique<TokenParse<R"(Bool\b)",          "BOOL",        BOOL>>());
    parsers_.push_back(std::make_unique<TokenParse<R"(String\b)",        "STRING",      STRING>>());
    parsers_.push_back(std::make_unique<TokenParse<R"(cast\b)",          "CAST",        CAST>>());
    parsers_.push_back(std::make_unique<TokenParse<R"(struct\b)",        "STRUCT",      STRUCT>>());

    // spaces, comments, errors(comments) and strings
    parsers_.push_back(std::make_unique<TokenParse<R"(\s+)",             "SPACE",       SPACE>>());
    parsers_.push_back(std::make_unique<TokenParse<R"(//[^\n]*\n)",      "SINGLE_COMM", SINGLE_COMM>>());
    parsers_.push_back(std::make_unique<TokenParse<R"(/\*.*?\*/)",       "MULTI_COMM",  MULTI_COMM>>());
    parsers_.push_back(std::make_unique<TokenParse<R"(//.*?$)",          "ERROR",       ERROR, "Unterminated single-line comment">>());
    parsers_.push_back(std::make_unique<TokenParse<R"(/\*.*?$)",         "ERROR",       ERROR, "Unterminated multi-line comment">>());
    parsers_.push_back(std::make_unique<TokenParse<R"("([^"\\]|\\.)*")", "STR",         STR>>());
    parsers_.push_back(std::make_unique<TokenParse<R"('([^'\\]|\\.)*')", "STR",         STR>>());

    // number and ident
    parsers_.push_back(std::make_unique<TokenParse<R"(\d+)",             "INT",         INT>>());
    parsers_.push_back(std::make_unique<TokenParse<R"([a-zA-Z_]\w*)",    "IDENT",       IDENT>>());

    // brackets
    parsers_.push_back(std::make_unique<TokenParse<R"(\()",              "LPAREN",      LPAREN>>());
    parsers_.push_back(std::make_unique<TokenParse<R"(\))",              "RPAREN",      RPAREN>>());
    parsers_.push_back(std::make_unique<TokenParse<R"(\{)",              "LBRACE",      LBRACE>>());
    parsers_.push_back(std::make_unique<TokenParse<R"(\})",              "RBRACE",      RBRACE>>());
    parsers_.push_back(std::make_unique<TokenParse<R"(\[)",              "LBRACKET",    LBRACKET>>());
    parsers_.push_back(std::make_unique<TokenParse<R"(\])",              "RBRACKET",    RBRACKET>>());
    
    // operatitons
    parsers_.push_back(std::make_unique<TokenParse<R"(&&)",              "AND",         AND>>());
    parsers_.push_back(std::make_unique<TokenParse<R"(\|\|)",            "OR",          OR>>());
    parsers_.push_back(std::make_unique<TokenParse<R"(<=)",              "LE",          LE>>());
    parsers_.push_back(std::make_unique<TokenParse<R"(>=)",              "GE",          GE>>());
    parsers_.push_back(std::make_unique<TokenParse<R"(!=)",              "NE",          NE>>());
    parsers_.push_back(std::make_unique<TokenParse<R"(==)",              "EQ",          EQ>>());
    parsers_.push_back(std::make_unique<TokenParse<R"(=)",               "ASSIGN",      ASSIGN>>());
    parsers_.push_back(std::make_unique<TokenParse<R"(<)",               "LT",          LT>>());
    parsers_.push_back(std::make_unique<TokenParse<R"(>)",               "GT",          GT>>());
    parsers_.push_back(std::make_unique<TokenParse<R"(!)",               "NOT",         NOT>>());
    parsers_.push_back(std::make_unique<TokenParse<R"(\+)",              "PLUS",        PLUS>>());
    parsers_.push_back(std::make_unique<TokenParse<R"(-)",               "MINUS",       MINUS>>());
    parsers_.push_back(std::make_unique<TokenParse<R"(\*)",              "MULT",        MULTI>>());
    parsers_.push_back(std::make_unique<TokenParse<R"(/)",               "DIV",         DIVI>>());
    
    // dot, colon, comma and semicolon
    parsers_.push_back(std::make_unique<TokenParse<R"(\.)",              "DOT",         DOT>>());
    parsers_.push_back(std::make_unique<TokenParse<R"(:)",               "COLON",       COLON>>());
    parsers_.push_back(std::make_unique<TokenParse<R"(,)",               "COMMA",       COMMA>>());
    parsers_.push_back(std::make_unique<TokenParse<R"(;)",               "SEMI",        SEMI>>());
}


std::vector<Token> Lexer::parse(const std::string& src) const { 
    std::vector<Token> tokens;

    size_t offset = 0, line = 1, column = 1;
    while (offset != src.size()) {

        Token res;
        bool mathed = false;
        for (const auto& parser : parsers_) {
        
            auto t = parser->getToken(src, offset, line, column);
            if (t) {
                res = t.value();
                mathed = true;
                break;
            }
        }
        
        if (!mathed) { 
            res = Token(ERROR, src.substr(offset, 1), "ERROR", line, column);
            offset++;
            column++; 
        }
        tokens.push_back(res);
    }
    tokens.push_back(Token(END, "", "EOF", line, column));

    return tokens;
}

