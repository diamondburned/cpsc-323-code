#pragma once

#include <memory>
#include <stack>
#include <string>

enum Operator {
  OP_PLUS = '+',
  OP_MINUS = '-',
  OP_MULTIPLY = '*',
  OP_DIVIDE = '/',
  OP_END = '$',
};

struct Node {
  virtual ~Node() = default;
  virtual std::string to_string() const = 0;
};

struct Value : Node {
  std::string name;

  Value(char value) : name(std::string{value}) {}

  std::string to_string() const override { return name; }
};

struct Operation : Node {
  std::unique_ptr<Node> left;
  std::unique_ptr<Node> right;
  Operator op;

  Operation(std::unique_ptr<Node>& left_, std::unique_ptr<Node>& right_,
            Operator op_)
      : left(std::move(left_)), right(std::move(right_)), op(op_) {}

  std::string to_string() const override {
    return "(" + left->to_string() + " " + right->to_string() + " " +
           std::string{static_cast<char>(op)} + ")";
  }
};

std::unique_ptr<Node> parse(const std::string& input);
