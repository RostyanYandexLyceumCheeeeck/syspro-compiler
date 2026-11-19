#pragma once

#include <memory>
#include <vector>
#include <string>
#include <string_view>

#include <Token.hpp>
#include <DeclarationsUp.hpp>


class Parser {
    std::vector<Token> tokens_;
    size_t pos_ = 0;
    bool hasError_ = false;
    std::string grammar_;

    // navigation on stream tokens 
    const Token& peek(size_t offset = 0) const;
    const Token& advance();
    bool isAtEnd() const;
    bool check(TokenType type, size_t offset = 0) const;
    bool match(TokenType type);
    const Token* consume(TokenType type, std::string_view errMsg);

    // Panic Mode
    void reportError(const Token& tok, std::string_view msg);
    void synchronize();

    // parse instructions
    StmtPtr parseStatement();
    StmtPtr parseReturnStatement();
    StmtPtr parseDeclarationStatement();
    StmtPtr parseAssignmentStatement();
    StmtPtr parseExpressionStatement();

    std::unique_ptr<BlockStmt> parseBlock();
    StmtPtr parseIfStatement();
    StmtPtr parseWhileStatement();
    StmtPtr parseBreakStatement();
    StmtPtr parseContinueStatement();

    // parse expressions
    ExprPtr parseExpression();
    ExprPtr parseAdditiveExpression();
    ExprPtr parseMultiplicativeExpression();
    ExprPtr parseUnaryExpression();
    ExprPtr parsePrimaryExpression();
    
    ExprPtr parseLogicalOrExpression();
    ExprPtr parseLogicalAndExpression();
    ExprPtr parseEqualityExpression();
    ExprPtr parseRelationalExpression();
    ExprPtr parseCallExpression();
    bool parseArgumentList(std::vector<ExprPtr>& args);

    // parse declarations levelUp
    ExternPtr parseExternDeclaration();
    FuncPtr parseFuncDeclaration();
    bool parseParamList(std::vector<IdentPtr>& params);

public:
    explicit Parser(const std::vector<Token>& rawTokens, std::string grammar = "1");
    static std::vector<Token> loadTokensFromJson(const std::string& filePath);

    std::unique_ptr<Program> parseProgram();
    bool hasError() const { return hasError_; }
};