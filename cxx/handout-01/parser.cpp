#include "parser.hpp"

#include <memory>

std::unique_ptr<Node> parse(const std::string &input) {
  std::stack<std::unique_ptr<Node>> stack;
  for (const auto &word : input) {
    switch (word) {
      case OP_PLUS:
      case OP_MINUS:
      case OP_MULTIPLY:
      case OP_DIVIDE: {
        auto r = std::move(stack.top());
        stack.pop();
        auto l = std::move(stack.top());
        stack.pop();

        stack.push(std::make_unique<Operation>(l, r, Operator(word)));
        break;
      }
      case OP_END: {
        goto done;
      }
      default: {
        stack.push(std::make_unique<Value>(word));
      }
    }
  }
done:
  return std::move(stack.top());
}
