#include <fstream>
#include <iostream>

#include "lexer.hpp"

int main(int argc, char* argv[]) {
  std::string inName = "-";
  if (argc > 1) {
    inName = std::string(argv[1]);
  }

  std::istream* in;
  if (inName == "-") {
    in = &std::cin;
  } else {
    in = new std::ifstream(inName);
  }

  auto statements = Lexer::lex(*in);
  Lexer::print(std::cout, statements);
}
