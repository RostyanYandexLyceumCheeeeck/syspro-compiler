#pragma once

#include <Statements.hpp>


class ExternDecl : public AstNode {
    std::string name_;
    std::vector<IdentPtr> params_;

public:
    ExternDecl(std::string name, 
               std::vector<IdentPtr> params, 
               size_t line, 
               size_t column): AstNode(line, column), 
                               name_(std::move(name)), 
                               params_(std::move(params)) {}

    const std::string& getName() const { return name_; }
    const std::vector<IdentPtr>& getParams() const { return params_; }

    nlohmann::ordered_json toJson() const override {
        nlohmann::ordered_json res;
        
        res["kind"] = "ExternDecl";
        res["name"] = name_;
        res["params"] = nlohmann::ordered_json::array();
        res["line"] = line_;
        res["column"] = column_;

        for (const auto& p : params_) {
            if (p) res["params"].push_back(p->toJson());
        }
        return res;
    }
};


class FuncDecl : public AstNode {
    std::string name_;
    std::vector<IdentPtr> params_;
    std::unique_ptr<BlockStmt> body_;

public:
    FuncDecl(std::string name, 
             std::vector<IdentPtr> params,
             std::unique_ptr<BlockStmt> body,
             size_t line,
             size_t column): AstNode(line, column),
                             name_(std::move(name)),
                             params_(std::move(params)),
                             body_(std::move(body)) {}

    const BlockStmt* getBody() const { return body_.get(); }
    const std::string& getName() const { return name_; }
    const std::vector<IdentPtr>& getParams() const { return params_; }

    nlohmann::ordered_json toJson() const override {
        nlohmann::ordered_json res;
        
        res["kind"] = "FuncDecl";
        res["name"] = name_;
        res["params"] = nlohmann::ordered_json::array();
        res["body"] = body_ ? body_->toJson() : nullptr;
        res["line"] = line_;
        res["column"] = column_;

        for (const auto& p : params_) {
            if (p) res["params"].push_back(p->toJson());
        }
        return res;
    }
};

using ExternPtr = std::unique_ptr<ExternDecl>;
using FuncPtr = std::unique_ptr<FuncDecl>;


class Program : public AstNode {
    std::vector<AstNodePtr> body_;

public:
    Program(std::vector<AstNodePtr> body, size_t line = 1, size_t column = 1)
        : AstNode(line, column), body_(std::move(body)) {}

    const std::vector<AstNodePtr>& getBody() const { return body_; }

    nlohmann::ordered_json toJson() const override {
        nlohmann::ordered_json res;
        
        res["kind"] = "Program";
        res["body"] = nlohmann::ordered_json::array();
        res["line"] = line_;
        res["column"] = column_;
        
        for (const auto& node : body_) {
            if (node) { res["body"].push_back(node->toJson()); }
        }
        return res;
    }
};