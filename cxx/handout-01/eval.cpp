#include "eval.hpp"

#include <stdexcept>
#include <unordered_map>

#include "parser.hpp"

struct evaluator {
  const std::unordered_map<std::string, int>& variables;

  int evaluate(const std::unique_ptr<Node>& node) {
    if (auto* value = dynamic_cast<Value*>(node.get())) {
      return variables.at(value->name);
    }
    if (auto* value = dynamic_cast<Operation*>(node.get())) {
      const auto l = evaluate(value->left);
      const auto r = evaluate(value->right);
      switch (value->op) {
        case OP_PLUS:
          return l + r;
        case OP_MINUS:
          return l - r;
        case OP_MULTIPLY:
          return l * r;
        case OP_DIVIDE:
          return l / r;
        case OP_END:
          throw std::runtime_error("Unexpected end of expression");
      }
    }
    throw std::domain_error("Unknown node type");
  }
};

int eval(const std::unique_ptr<Node>& node,
         const std::unordered_map<std::string, int>& variables) {
  evaluator e{variables};
  return e.evaluate(node);
}
