#include <iostream>
#include <unordered_map>

#include "eval.hpp"
#include "parser.hpp"

int main() {
  const std::unordered_map<std::string, int> variables({
      {"a", 5},
      {"b", 7},
      {"c", 2},
      {"d", 4},
  });

  while (true) {
    std::string expr;
    std::cout << "Enter a postfix expression with $ at the end: ";
    std::cin >> expr;

    try {
      auto node = parse(expr);
      auto value = eval(node, variables);
      std::cout << "\tValue = " << value << std::endl;
    } catch (const std::exception& e) {
      std::cout << "\tError: " << e.what() << std::endl;
    }

    std::string choice;
    std::cout << "Continue(Y/n)? ";
    std::cin >> choice;

    if (choice == "n") {
      break;
    }
  }
}
