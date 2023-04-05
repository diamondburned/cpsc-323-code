#include <fstream>
#include <iostream>

#include "lib/lexer.hpp"

int main(int argc, char* argv[]) {
  if (argc != 2) {
    std::cerr << "usage: " << argv[0] << " filename" << std::endl;
    return 1;
  }

  std::ifstream in(argv[1]);
  if (!in) {
    std::cerr << "error: could not open file " << argv[1] << std::endl;
    return 1;
  }

  auto lines = Lexer::lex(in);
  std::cout << Lexer::removeComments(lines) << std::endl;
}
