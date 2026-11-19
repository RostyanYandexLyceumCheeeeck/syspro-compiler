#pragma once

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <Ast.hpp>


class Expression : public AstNode {
public:
    using AstNode::AstNode;
};

using ExprPtr = std::unique_ptr<Expression>;


class IntegerLiteralExpr : public Expression {
    int64_t value_;

public:
    IntegerLiteralExpr(int64_t value, size_t line, size_t column): Expression(line, column), value_(value) {}

    int64_t getValue() const { return value_; }

    nlohmann::ordered_json toJson() const override {
        nlohmann::ordered_json res;
        
        res["kind"] = "IntLiteral";
        res["value"] = value_;
        res["line"] = line_;
        res["column"] = column_;
        
        return res;
    }
};


class IdentifierExpr : public Expression {
    std::string name_;

public:
    IdentifierExpr(std::string name, 
                   size_t line, 
                   size_t column): Expression(line, column), 
                                   name_(std::move(name)) {}

    const std::string& getName() const { return name_; }

    nlohmann::ordered_json toJson() const override {
        nlohmann::ordered_json res;
        
        res["kind"] = "Ident";
        res["name"] = name_;
        res["line"] = line_;
        res["column"] = column_;
        
        return res;
    }
};

using IdentPtr = std::unique_ptr<IdentifierExpr>;


class UnaryExpr : public Expression {
    std::string op_;
    ExprPtr operand_;

public:
    UnaryExpr(std::string op, 
              ExprPtr operand, 
              size_t line, 
              size_t column): Expression(line, column),
                              op_(std::move(op)),
                              operand_(std::move(operand)) {}

    const std::string& getOp() const { return op_; }
    const Expression* getOperand() const { return operand_.get(); }

    nlohmann::ordered_json toJson() const override {
        nlohmann::ordered_json res;
        
        res["kind"] = "UnaryOp";
        res["op"] = op_;
        res["operand"] = operand_ ? operand_->toJson() : nullptr;
        res["line"] = line_;
        res["column"] = column_;
        
        return res;
    }
};


class BinaryExpr : public Expression {
    std::string op_;
    ExprPtr lhs_;
    ExprPtr rhs_;

public:
    BinaryExpr(std::string op, 
               ExprPtr lhs, 
               ExprPtr rhs, 
               size_t line, 
               size_t column): Expression(line, column),
                               op_(std::move(op)),
                               lhs_(std::move(lhs)),
                               rhs_(std::move(rhs)) {}

    const std::string& getOp() const { return op_; }
    const Expression* getLhs() const { return lhs_.get(); }
    const Expression* getRhs() const { return rhs_.get(); }

    nlohmann::ordered_json toJson() const override {
        nlohmann::ordered_json res;
        
        res["kind"] = "BinOp";
        res["op"] = op_;
        res["left"] = lhs_ ? lhs_->toJson() : nullptr;
        res["right"] = rhs_ ? rhs_->toJson() : nullptr;
        res["line"] = line_;
        res["column"] = column_;
        
        return res;
    }
};


class BoolLiteralExpr : public Expression {
    bool value_;

public:
    BoolLiteralExpr(bool value, size_t line, size_t column): Expression(line, column), value_(value) {}

    bool getValue() const { return value_; }

    nlohmann::ordered_json toJson() const override {
        nlohmann::ordered_json res;
        
        res["kind"] = "BoolLiteral";
        res["value"] = value_;
        res["line"] = line_;
        res["column"] = column_;
        
        return res;
    }
};


class CallExpr : public Expression {
    ExprPtr callee_;
    std::vector<ExprPtr> args_;

public:
    CallExpr(ExprPtr callee, 
             std::vector<ExprPtr> args, 
             size_t line, 
             size_t column): Expression(line, column),
                             callee_(std::move(callee)),
                             args_(std::move(args)) {}

    const Expression* getCallee() const { return callee_.get(); }
    const std::vector<ExprPtr>& getArgs() const { return args_; }

    nlohmann::ordered_json toJson() const override {
        nlohmann::ordered_json res;
        
        res["kind"] = "Call";
        res["callee"] = callee_ ? callee_->toJson() : nullptr;
        res["args"] = nlohmann::ordered_json::array();
        res["line"] = line_;
        res["column"] = column_;
        
        for (const auto& arg : args_) {
            if (arg) { res["args"].push_back(arg->toJson()); }
        }
        return res;
    }
};

