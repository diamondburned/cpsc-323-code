#include <fstream>
#include <iostream>

#include "lib/grammar.hpp"
#include "lib/lexer.hpp"
#include "lib/parser.hpp"
#include "lib/transpile.hpp"

int main(int argc, char* argv[]) {
  if (argc != 2) {
    std::cerr << "usage: " << argv[0] << " program_file" << std::endl;
    return 1;
  }

  std::string inputPath = argv[1];

  std::ifstream in(inputPath);
  if (!in) {
    std::cerr << "error: could not open file " << argv[1] << std::endl;
    return 1;
  }

  auto file = Lexer::lex(in);
  file = file.removeComments();

  std::ofstream stage1(inputPath + ".1.txt");
  stage1 << file << std::endl;
  stage1.close();

  Grammar grammar("grammar.txt");

  Parser parser(grammar);
  parser.loadErrorEntries("error-entry-messages.txt");

  Parser::Program program = parser.parse(file);

  std::ofstream stage2(inputPath + ".2.txt");
  stage2 << program << std::endl;
  stage2.close();

  std::ofstream stage3(inputPath + ".3.cpp");
  CTranspiler::transpile(stage3, program);
  stage3.close();
}
