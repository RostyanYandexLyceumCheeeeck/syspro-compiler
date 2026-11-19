#include <charconv>
#include <fstream>
#include <iostream>

#include <Parser.hpp>


Parser::Parser(const std::vector<Token>& rawTokens, std::string grammar): grammar_(std::move(grammar)) {
    tokens_.reserve(rawTokens.size());
    
    for (const auto& tok : rawTokens) {
        auto type = tok.getType();
        if (type == TokenType::SPACE ||
            type == TokenType::SINGLE_COMM ||
            type == TokenType::MULTI_COMM) {
            continue;
        }
        tokens_.push_back(tok);
    }

    if (tokens_.empty() || tokens_.back().getType() != TokenType::END) {
        tokens_.emplace_back(TokenType::END, "", 1, 1);
    }
}


std::vector<Token> Parser::loadTokensFromJson(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) { return {}; }

    nlohmann::json j;
    file >> j;

    std::vector<Token> tokens;
    if (j.is_array()) {
        for (const auto& item : j) {
            std::string kind = item.value("kind", "UNKNOWN");
            std::string value = item.value("value", "");
            size_t line = item.value("line", 1);
            size_t column = item.value("column", 1);

            tokens.emplace_back(tokenTypeFromString(kind), std::move(value), line, column);
        }
    }

    if (tokens.empty() || tokens.back().getType() != TokenType::END) {
        size_t l = tokens.empty() ? 1 : tokens.back().getLine();
        size_t c = tokens.empty() ? 1 : tokens.back().getColumn();
        tokens.emplace_back(TokenType::END, "", l, c);
    }

    return tokens;
}


const Token& Parser::peek(size_t offset) const {
    if (pos_ + offset >= tokens_.size()) { return tokens_.back(); }
    return tokens_[pos_ + offset];
}


const Token& Parser::advance() {
    if (!isAtEnd()) { ++pos_; }
    return tokens_[pos_ - 1];
}


bool Parser::isAtEnd() const {
    return peek().getType() == TokenType::END;
}


bool Parser::check(TokenType type, size_t offset) const {
    return peek(offset).getType() == type;
}


bool Parser::match(TokenType type) {
    if (check(type)) {
        advance();
        return true;
    }
    return false;
}


const Token* Parser::consume(TokenType type, std::string_view errMsg) {
    if (check(type)) { return &advance(); }
    reportError(peek(), errMsg);
    return nullptr;
}


void Parser::reportError(const Token& tok, std::string_view msg) {
    hasError_ = true;
    std::cerr << "Parse error at line " << tok.getLine()
              << ", column " << tok.getColumn()
              << " (token '" << tok.getValue() << "'): "
              << msg << "\n";
}


void Parser::synchronize() {
    while (!isAtEnd()) {
        TokenType type = peek().getType();
        if (type == TokenType::SEMI) {
            advance(); // ;
            return;
        }

        if (type == TokenType::RBRACE) { return; }
        advance();
    }
}


// program ::= { externDeclaration | funcDeclaration | statement } EOF
std::unique_ptr<Program> Parser::parseProgram() {
    std::vector<AstNodePtr> body;
    bool hasMain = false;

    while (!isAtEnd()) {
        size_t startPos = pos_;

        if (check(TokenType::EXTERN)) {
            if (auto ext = parseExternDeclaration()) {
                body.push_back(std::move(ext));
            }
            
        } else if (check(TokenType::DEF)) {
            if (auto fn = parseFuncDeclaration()) {
                if (fn->getName() == "main") { hasMain = true; }
                body.push_back(std::move(fn));
            }

        } else if (check(TokenType::RBRACE)) {
            reportError(peek(), "Unexpected '}' at top level");
            advance();
        
        } else {
            body.push_back(parseStatement());
        }

        if (pos_ == startPos && !isAtEnd()) { advance(); }
    }

    const Token& eofTok = peek();

    if (grammar_ >= "3") {
        if (!hasMain) {
            reportError(eofTok, "Grammar 3 requires a 'main' function");
        }

    } else if (body.empty()) {
        reportError(eofTok, "Program must contain at least one statement before EOF");
    }

    size_t progCol = 1;
    size_t progLine = body.empty() ? eofTok.getLine() : body.back()->getLine();

    return std::make_unique<Program>(std::move(body), progLine, progCol);
}


// paramList ::= IDENT { "," IDENT }
bool Parser::parseParamList(std::vector<IdentPtr>& params) {
    if (check(TokenType::RPAREN)) { return true; } // epmty list

    const Token* first = consume(TokenType::IDENT, "Expected parameter name");
    if (!first) { return false; }
    
    params.push_back(std::make_unique<IdentifierExpr>(
        first->getValue(), first->getLine(), first->getColumn()
    ));

    while (match(TokenType::COMMA)) {
        const Token* next = consume(TokenType::IDENT, "Expected parameter name after ','");
        if (!next) { return false; }
        
        params.push_back(std::make_unique<IdentifierExpr>(
            next->getValue(), next->getLine(), next->getColumn()
        ));
    }

    return true;
}


// externDeclaration ::= "extern" "def" IDENT "(" [ paramList ] ")" ";"
ExternPtr Parser::parseExternDeclaration() {
    advance(); // extern

    if (!consume(TokenType::DEF, "Expected 'def' after 'extern'")) {
        synchronize();
        return nullptr;
    }

    const Token* nameTok = consume(TokenType::IDENT, "Expected function name in extern declaration");
    if (!nameTok || !consume(TokenType::LPAREN, "Expected '(' after function name")) {
        synchronize();
        return nullptr;
    }

    std::vector<IdentPtr> params;
    if (!parseParamList(params) || !consume(TokenType::RPAREN, "Expected ')' after parameter list")) {
        synchronize();
        return nullptr;
    }

    const Token* semiTok = consume(TokenType::SEMI, "Expected ';' after extern declaration");
    if (!semiTok) {
        synchronize();
        return nullptr;
    }

    return std::make_unique<ExternDecl>(
        nameTok->getValue(), std::move(params), semiTok->getLine(), semiTok->getColumn()
    );
}

// funcDeclaration ::= "def" IDENT "(" [ paramList ] ")" block
FuncPtr Parser::parseFuncDeclaration() {
    advance(); // def

    const Token* nameTok = consume(TokenType::IDENT, "Expected function name after 'def'");
    if (!nameTok || !consume(TokenType::LPAREN, "Expected '(' after function name")) {
        synchronize();
        if (check(TokenType::RBRACE)) { advance(); }

        return nullptr;
    }

    std::vector<IdentPtr> params;
    if (!parseParamList(params) || !consume(TokenType::RPAREN, "Expected ')' after parameters")) {
        synchronize();
        if (check(TokenType::RBRACE)) { advance(); }
        
        return nullptr;
    }

    if (!check(TokenType::LBRACE)) {
        reportError(peek(), "Expected '{' to start function body");
        synchronize();
        if (check(TokenType::RBRACE)) { advance(); }
        
        return nullptr;
    }

    auto body = parseBlock();
    size_t endLine = body ? body->getLine() : nameTok->getLine();
    size_t endCol = body ? body->getColumn() : nameTok->getColumn();

    return std::make_unique<FuncDecl>(
        nameTok->getValue(), std::move(params), std::move(body), endLine, endCol
    );
}


StmtPtr Parser::parseStatement() {
    const Token& cur = peek();

    if (cur.getType() == TokenType::ERROR) {
        reportError(cur, "Lexical error token encountered");
        synchronize();
        return std::make_unique<ErrorStmt>(cur.getLine(), cur.getColumn());
    }

    if (check(TokenType::IF)) { return parseIfStatement(); }
    if (check(TokenType::WHILE)) { return parseWhileStatement(); }
    if (check(TokenType::BREAK)) { return parseBreakStatement(); }
    if (check(TokenType::RETURN)) { return parseReturnStatement(); }
    if (check(TokenType::LBRACE)) { return parseBlock(); }
    if (check(TokenType::CONTINUE)) { return parseContinueStatement(); }
    
    if (check(TokenType::VAL) || check(TokenType::VAR)) { return parseDeclarationStatement(); }

    if (check(TokenType::IDENT) && check(TokenType::ASSIGN, 1)) {
        return parseAssignmentStatement();
    }

    return parseExpressionStatement();
}


// returnStatement ::= "return" expression ";"
StmtPtr Parser::parseReturnStatement() {
    const Token& kw = advance(); // return

    ExprPtr expr = parseExpression();
    if (!expr) {
        synchronize();
        return std::make_unique<ErrorStmt>(kw.getLine(), kw.getColumn());
    }
    
    const Token* semiTok = consume(TokenType::SEMI, "Expected ';' after return statement");
    if (!semiTok) {
        synchronize();
        return std::make_unique<ErrorStmt>(kw.getLine(), kw.getColumn());
    }

    return std::make_unique<ReturnStmt>(std::move(expr), semiTok->getLine(), semiTok->getColumn());
}


// declarationStatement ::= ("val" | "var") IDENT "=" expression ";"
StmtPtr Parser::parseDeclarationStatement() {
    const Token& kw = advance(); // val or var

    const Token* ident = consume(TokenType::IDENT, "Expected identifier in declaration");
    if (!ident) {
        synchronize();
        return std::make_unique<ErrorStmt>(kw.getLine(), kw.getColumn());
    }

    if (!consume(TokenType::ASSIGN, "Expected '=' after identifier in declaration")) {
        synchronize();
        return std::make_unique<ErrorStmt>(kw.getLine(), kw.getColumn());
    }

    ExprPtr init = parseExpression();
    if (!init) {
        synchronize();
        return std::make_unique<ErrorStmt>(kw.getLine(), kw.getColumn());
    }
    
    const Token* semiTok = consume(TokenType::SEMI, "Expected ';' after declaration");
    if (!semiTok) {
        synchronize();
        return std::make_unique<ErrorStmt>(kw.getLine(), kw.getColumn());
    }

    bool isMutable = (kw.getType() == TokenType::VAR);
    return std::make_unique<DeclStmt>(
        isMutable, ident->getValue(), std::move(init), semiTok->getLine(), semiTok->getColumn()
    );
}


// assignmentStatement ::= IDENT "=" expression ";"
StmtPtr Parser::parseAssignmentStatement() {
    const Token& ident = advance(); // IDENT
    advance();                      // =

    ExprPtr value = parseExpression();
    if (!value) {
        synchronize();
        return std::make_unique<ErrorStmt>(ident.getLine(), ident.getColumn());
    }
    
    const Token* semiTok = consume(TokenType::SEMI, "Expected ';' after assignment");
    if (!semiTok) {
        synchronize();
        return std::make_unique<ErrorStmt>(ident.getLine(), ident.getColumn());
    }

    return std::make_unique<AssignStmt>(
        ident.getValue(), std::move(value), semiTok->getLine(), semiTok->getColumn()
    );
}

// expressionStatement ::= expression ";"
StmtPtr Parser::parseExpressionStatement() {
    const Token& startTok = peek();
    ExprPtr expr = parseExpression();
    if (!expr) {
        synchronize();
        return std::make_unique<ErrorStmt>(startTok.getLine(), startTok.getColumn());
    }

    const Token* semiTok = consume(TokenType::SEMI, "Expected ';' after expression");
    if (!semiTok) {
        synchronize();
        return std::make_unique<ErrorStmt>(startTok.getLine(), startTok.getColumn());
    }

    return std::make_unique<ExprStmt>(std::move(expr), semiTok->getLine(), semiTok->getColumn());
}


// block ::= "{" { statement } "}"
std::unique_ptr<BlockStmt> Parser::parseBlock() {
    const Token& lbrace = advance(); // {

    std::vector<StmtPtr> statements;
    while (!check(TokenType::RBRACE) && !isAtEnd()) {
        size_t startPos = pos_;
        statements.push_back(parseStatement());
        if (pos_ == startPos && !isAtEnd() && !check(TokenType::RBRACE)) { advance(); }
    }

    const Token* rbrace = consume(TokenType::RBRACE, "Expected '}' after block");
    size_t endLine = rbrace ? rbrace->getLine() : lbrace.getLine();
    size_t endCol = rbrace ? rbrace->getColumn() : lbrace.getColumn();

    return std::make_unique<BlockStmt>(std::move(statements), endLine, endCol);
}


// ifStatement ::= "if" "(" expression ")" statement [ "else" statement ]
StmtPtr Parser::parseIfStatement() {
    const Token& kw = advance(); // if

    if (!consume(TokenType::LPAREN, "Expected '(' after 'if'")) {
        synchronize();
        return std::make_unique<ErrorStmt>(kw.getLine(), kw.getColumn());
    }

    ExprPtr cond = parseExpression();
    if (!cond || !consume(TokenType::RPAREN, "Expected ')' after if condition")) {
        synchronize();
        return std::make_unique<ErrorStmt>(kw.getLine(), kw.getColumn());
    }

    StmtPtr thenBranch = parseStatement();
    StmtPtr elseBranch = nullptr;

    if (match(TokenType::ELSE)) { elseBranch = parseStatement(); }

    size_t endLine = elseBranch ? elseBranch->getLine() : (thenBranch ? thenBranch->getLine() : kw.getLine());
    size_t endCol = elseBranch ? elseBranch->getColumn() : (thenBranch ? thenBranch->getColumn() : kw.getColumn());

    return std::make_unique<IfStmt>(
        std::move(cond), std::move(thenBranch), std::move(elseBranch), endLine, endCol
    );
}


// whileStatement ::= "while" "(" expression ")" statement
StmtPtr Parser::parseWhileStatement() {
    const Token& kw = advance(); // while

    if (!consume(TokenType::LPAREN, "Expected '(' after 'while'")) {
        synchronize();
        return std::make_unique<ErrorStmt>(kw.getLine(), kw.getColumn());
    }

    ExprPtr cond = parseExpression();
    if (!cond || !consume(TokenType::RPAREN, "Expected ')' after while condition")) {
        synchronize();
        return std::make_unique<ErrorStmt>(kw.getLine(), kw.getColumn());
    }

    StmtPtr body = parseStatement();
    size_t endLine = body ? body->getLine() : kw.getLine();
    size_t endCol = body ? body->getColumn() : kw.getColumn();

    return std::make_unique<WhileStmt>(std::move(cond), std::move(body), endLine, endCol);
}

// breakStatement ::= "break" ";"
StmtPtr Parser::parseBreakStatement() {
    const Token& kw = advance(); // break
    
    const Token* semiTok = consume(TokenType::SEMI, "Expected ';' after 'break'");
    if (!semiTok) {
        synchronize();
        return std::make_unique<ErrorStmt>(kw.getLine(), kw.getColumn());
    }

    return std::make_unique<BreakStmt>(semiTok->getLine(), semiTok->getColumn());
}

// continueStatement ::= "continue" ";"
StmtPtr Parser::parseContinueStatement() {
    const Token& kw = advance(); // continue

    const Token* semiTok = consume(TokenType::SEMI, "Expected ';' after 'continue'");
    if (!semiTok) {
        synchronize();
        return std::make_unique<ErrorStmt>(kw.getLine(), kw.getColumn());
    }
    
    return std::make_unique<ContinueStmt>(semiTok->getLine(), semiTok->getColumn());
}


// expression ::= logicalOrExpression
ExprPtr Parser::parseExpression() {
    return parseLogicalOrExpression();
}


// logicalOrExpression ::= logicalAndExpression { "||" logicalAndExpression }
ExprPtr Parser::parseLogicalOrExpression() {
    ExprPtr lhs = parseLogicalAndExpression();
    if (!lhs) { return nullptr; }

    while (check(TokenType::OR)) {
        const Token& opTok = advance();
        ExprPtr rhs = parseLogicalAndExpression();
        if (!rhs) { return nullptr; }
        
        size_t endLine = rhs->getLine();
        size_t endCol = rhs->getColumn();
        lhs = std::make_unique<BinaryExpr>(
            opTok.getValue(), std::move(lhs), std::move(rhs), endLine, endCol
        );
    }

    return lhs;
}


// logicalAndExpression ::= equalityExpression { "&&" equalityExpression }
ExprPtr Parser::parseLogicalAndExpression() {
    ExprPtr lhs = parseEqualityExpression();
    if (!lhs) { return nullptr; }

    while (check(TokenType::AND)) {
        const Token& opTok = advance();
        ExprPtr rhs = parseEqualityExpression();
        if (!rhs) { return nullptr; }

        size_t endLine = rhs->getLine();
        size_t endCol = rhs->getColumn();
        lhs = std::make_unique<BinaryExpr>(
            opTok.getValue(), std::move(lhs), std::move(rhs), endLine, endCol
        );
    }

    return lhs;
}


// equalityExpression ::= relationalExpression { ("==" | "!=") relationalExpression }
ExprPtr Parser::parseEqualityExpression() {
    ExprPtr lhs = parseRelationalExpression();
    if (!lhs) { return nullptr; }

    while (check(TokenType::EQ) || check(TokenType::NE)) {
        const Token& opTok = advance();
        ExprPtr rhs = parseRelationalExpression();
        if (!rhs) { return nullptr; }
        
        size_t endLine = rhs->getLine();
        size_t endCol = rhs->getColumn();
        lhs = std::make_unique<BinaryExpr>(
            opTok.getValue(), std::move(lhs), std::move(rhs), endLine, endCol
        );
    }

    return lhs;
}


// relationalExpression ::= additiveExpression { ("<" | ">" | "<=" | ">=") additiveExpression }
ExprPtr Parser::parseRelationalExpression() {
    ExprPtr lhs = parseAdditiveExpression();
    if (!lhs) { return nullptr; }

    while (check(TokenType::LT) || check(TokenType::GT) ||
           check(TokenType::LE) || check(TokenType::GE)) {
        
        const Token& opTok = advance();
        ExprPtr rhs = parseAdditiveExpression();
        if (!rhs) { return nullptr; }
        
        size_t endLine = rhs->getLine();
        size_t endCol = rhs->getColumn();
        lhs = std::make_unique<BinaryExpr>(
            opTok.getValue(), std::move(lhs), std::move(rhs), endLine, endCol
        );
    }

    return lhs;
}


// additiveExpression ::= multiplicativeExpression { ("+" | "-") multiplicativeExpression }
ExprPtr Parser::parseAdditiveExpression() {
    ExprPtr lhs = parseMultiplicativeExpression();
    if (!lhs) { return nullptr; }

    while (check(TokenType::PLUS) || check(TokenType::MINUS)) {
        const Token& opTok = advance();
        ExprPtr rhs = parseMultiplicativeExpression();
        if (!rhs) { return nullptr; }

        size_t endLine = rhs->getLine();
        size_t endCol = rhs->getColumn();
        lhs = std::make_unique<BinaryExpr>(
            opTok.getValue(), std::move(lhs), std::move(rhs), endLine, endCol
        );
    }

    return lhs;
}


// multiplicativeExpression ::= unaryExpression { ("*" | "/") unaryExpression }
ExprPtr Parser::parseMultiplicativeExpression() {
    ExprPtr lhs = parseUnaryExpression();
    if (!lhs) { return nullptr; }

    while (check(TokenType::MULT) || check(TokenType::DIV)) {
        const Token& opTok = advance();
        ExprPtr rhs = parseUnaryExpression();
        if (!rhs) { return nullptr; }

        size_t endLine = rhs->getLine();
        size_t endCol = rhs->getColumn();
        lhs = std::make_unique<BinaryExpr>(
            opTok.getValue(), std::move(lhs), std::move(rhs), endLine, endCol
        );
    }

    return lhs;
}

// unaryExpression ::= "!" unaryExpression | "-" unaryExpression | callExpression
ExprPtr Parser::parseUnaryExpression() {
    if (check(TokenType::MINUS) || check(TokenType::NOT)) {
        const Token& opTok = advance();
        ExprPtr operand = parseUnaryExpression();
        if (!operand) { return nullptr; }

        size_t endLine = operand->getLine();
        size_t endCol = operand->getColumn();
        return std::make_unique<UnaryExpr>(
            opTok.getValue(), std::move(operand), endLine, endCol
        );
    }

    return parseCallExpression();
}


// callExpression ::= primaryExpression { "(" [ argumentList ] ")" }
ExprPtr Parser::parseCallExpression() {
    ExprPtr expr = parsePrimaryExpression();
    if (!expr) { return nullptr; }

    while (check(TokenType::LPAREN)) {
        advance(); // (
        std::vector<ExprPtr> args;
        if (!parseArgumentList(args)) { return nullptr; }

        const Token* rparen = consume(TokenType::RPAREN, "Expected ')' after function arguments");
        if (!rparen) { return nullptr; }

        expr = std::make_unique<CallExpr>(
            std::move(expr), std::move(args), rparen->getLine(), rparen->getColumn()
        );
    }

    return expr;
}

// argumentList ::= expression { "," expression }
bool Parser::parseArgumentList(std::vector<ExprPtr>& args) {
    if (check(TokenType::RPAREN)) { return true; } // empty list 

    ExprPtr first = parseExpression();
    if (!first) { return false; }

    args.push_back(std::move(first));

    while (match(TokenType::COMMA)) {
        ExprPtr next = parseExpression();
        if (!next) { return false; }

        args.push_back(std::move(next));
    }
    
    return true;
}

// primaryExpression ::= INTEGER_LITERAL | IDENT | "true" | "false" | "(" expression ")"
ExprPtr Parser::parsePrimaryExpression() {
    const Token& tok = peek();

    if (match(TokenType::INT)) {
        int64_t val = 0;
        const std::string& s = tok.getValue();
        auto [ptr, ec] = std::from_chars(s.data(), s.data() + s.size(), val);
        if (ec != std::errc{} || ptr != s.data() + s.size()) {
            reportError(tok, "Integer literal out of Int64 range");
            return nullptr;
        }

        return std::make_unique<IntegerLiteralExpr>(val, tok.getLine(), tok.getColumn());
    }

    if (match(TokenType::TRUE)) {
        return std::make_unique<BoolLiteralExpr>(true, tok.getLine(), tok.getColumn());
    }

    if (match(TokenType::FALSE)) {
        return std::make_unique<BoolLiteralExpr>(false, tok.getLine(), tok.getColumn());
    }

    if (match(TokenType::IDENT)) {
        return std::make_unique<IdentifierExpr>(tok.getValue(), tok.getLine(), tok.getColumn());
    }

    if (match(TokenType::LPAREN)) {
        ExprPtr inner = parseExpression();
        if (!inner) { return nullptr; }
        if (!consume(TokenType::RPAREN, "Expected ')' after expression")) { return nullptr; }
        
        return inner;
    }

    reportError(tok, "Expected primary expression");
    return nullptr;
}