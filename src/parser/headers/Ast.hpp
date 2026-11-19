#pragma once

#include <nlohmann/json.hpp>


class AstNode {
protected:
    size_t line_;
    size_t column_;

public:
    AstNode(size_t line, size_t column) : line_(line), column_(column) {}
    virtual ~AstNode() = default;

    size_t getLine() const { return line_; }
    size_t getColumn() const { return column_; }

    virtual nlohmann::ordered_json toJson() const = 0;
};

using AstNodePtr = std::unique_ptr<AstNode>;
