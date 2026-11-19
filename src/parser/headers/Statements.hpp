#pragma once

#include <cstdint>

#include <Expressions.hpp>


class Statement : public AstNode {
public:
    using AstNode::AstNode;
};

using StmtPtr = std::unique_ptr<Statement>;


class ReturnStmt : public Statement {
    ExprPtr expr_;

public:
    ReturnStmt(ExprPtr expr, size_t line, size_t column): Statement(line, column), expr_(std::move(expr)) {}

    const Expression* getExpr() const { return expr_.get(); }

    nlohmann::ordered_json toJson() const override {
        nlohmann::ordered_json res;

        res["kind"] = "Return";
        res["value"] = expr_ ? expr_->toJson() : nullptr;
        res["line"] = line_;
        res["column"] = column_;
        
        return res;
    }
};


class DeclStmt : public Statement {
    bool isMutable_; // true = var, false = val
    std::string name_;
    ExprPtr init_;

public:
    DeclStmt(bool isMutable, 
             std::string name, 
             ExprPtr init, 
             size_t line, 
             size_t column): Statement(line, column),
                             isMutable_(isMutable),
                             name_(std::move(name)),
                             init_(std::move(init)) {}

    bool isMutable() const { return isMutable_; }
    const std::string& getName() const { return name_; }
    const Expression* getInit() const { return init_.get(); }

    nlohmann::ordered_json toJson() const override {
        nlohmann::ordered_json res;
        
        res["kind"] = "Declare";
        res["mut"] = isMutable_ ? "var" : "val";
        res["name"] = name_;
        res["init"] = init_ ? init_->toJson() : nullptr;
        res["line"] = line_;
        res["column"] = column_;
        
        return res;
    }
};


class AssignStmt : public Statement {
    std::string name_;
    ExprPtr value_;

public:
    AssignStmt(std::string name, 
               ExprPtr value, 
               size_t line, 
               size_t column): Statement(line, column),
                               name_(std::move(name)),
                               value_(std::move(value)) {}

    const std::string& getName() const { return name_; }
    const Expression* getValue() const { return value_.get(); }

    nlohmann::ordered_json toJson() const override {
        nlohmann::ordered_json res;
        
        res["kind"] = "Assign";
        res["name"] = name_;
        res["value"] = value_ ? value_->toJson() : nullptr;
        res["line"] = line_;
        res["column"] = column_;
        
        return res;
    }
};


class ExprStmt : public Statement {
    ExprPtr expr_;

public:
    ExprStmt(ExprPtr expr, size_t line, size_t column): Statement(line, column), expr_(std::move(expr)) {}

    const Expression* getExpr() const { return expr_.get(); }

    nlohmann::ordered_json toJson() const override {
        nlohmann::ordered_json res;
        
        res["kind"] = "ExprStmt";
        res["value"] = expr_ ? expr_->toJson() : nullptr;
        res["line"] = line_;
        res["column"] = column_;
        
        return res;
    }
};


class BlockStmt : public Statement {
    std::vector<StmtPtr> statements_;

public:
    BlockStmt(std::vector<StmtPtr> statements, 
              size_t line, 
              size_t column): Statement(line, column), 
                              statements_(std::move(statements)) {}

    const std::vector<StmtPtr>& getStatements() const { return statements_; }

    nlohmann::ordered_json toJson() const override {
        nlohmann::ordered_json res;
  
        res["kind"] = "Block";
        res["body"] = nlohmann::ordered_json::array();
        res["line"] = line_;
        res["column"] = column_;
  
        for (const auto& stmt : statements_) {
            if (stmt) { res["body"].push_back(stmt->toJson()); }
        }
        return res;
    }
};


class IfStmt : public Statement {
    ExprPtr condition_;
    StmtPtr thenBranch_;
    StmtPtr elseBranch_; // nullptr <==> don't else

public:
    IfStmt(ExprPtr condition, 
           StmtPtr thenBranch, 
           StmtPtr elseBranch, 
           size_t line, 
           size_t column): Statement(line, column),
                           condition_(std::move(condition)),
                           thenBranch_(std::move(thenBranch)),
                           elseBranch_(std::move(elseBranch)) {}

    const Expression* getCondition() const { return condition_.get(); }
    const Statement* getThenBranch() const { return thenBranch_.get(); }
    const Statement* getElseBranch() const { return elseBranch_.get(); }

    nlohmann::ordered_json toJson() const override {
        nlohmann::ordered_json res;
        
        res["kind"] = "If";
        res["cond"] = condition_ ? condition_->toJson() : nullptr;
        res["then"] = thenBranch_ ? thenBranch_->toJson() : nullptr;
        res["else"] = elseBranch_ ? elseBranch_->toJson() : nullptr;
        res["line"] = line_;
        res["column"] = column_;
        
        return res;
    }
};


class WhileStmt : public Statement {
    ExprPtr condition_;
    StmtPtr body_;

public:
    WhileStmt(ExprPtr condition, 
              StmtPtr body, 
              size_t line, 
              size_t column): Statement(line, column),
                              condition_(std::move(condition)),
                              body_(std::move(body)) {}

    const Statement* getBody() const { return body_.get(); }
    const Expression* getCondition() const { return condition_.get(); }

    nlohmann::ordered_json toJson() const override {
        nlohmann::ordered_json res;

        res["kind"] = "While";
        res["cond"] = condition_ ? condition_->toJson() : nullptr;
        res["body"] = body_ ? body_->toJson() : nullptr;
        res["line"] = line_;
        res["column"] = column_;

        return res;
    }
};


class BreakStmt : public Statement {
public:
    using Statement::Statement;

    nlohmann::ordered_json toJson() const override {
        nlohmann::ordered_json res;
        
        res["kind"] = "Break";
        res["line"] = line_;
        res["column"] = column_;
        
        return res;
    }
};


class ContinueStmt : public Statement {
public:
    using Statement::Statement;

    nlohmann::ordered_json toJson() const override {
        nlohmann::ordered_json res;
        
        res["kind"] = "Continue";
        res["line"] = line_;
        res["column"] = column_;
        
        return res;
    }
};


class ErrorStmt : public Statement {
public:
    using Statement::Statement;

    nlohmann::ordered_json toJson() const override {
        nlohmann::ordered_json res;
        
        res["kind"] = "ErrorStmt";
        res["line"] = line_;
        res["column"] = column_;
        
        return res;
    }
};