#pragma once

#include <iostream>

#include "parser.hpp"

struct CTranspiler {
  class TranspileError;

  // transpile transpiles the given program to C++ and writes the result to out.
  static void transpile(std::ostream& out, const Parser::Program& program);
};

class CTranspiler::TranspileError : public std::runtime_error {
 public:
  const Parser::Program& program;  // the entire program
  const Parser::Token& token;      // where the error occurred

  TranspileError(const Parser::Program& program, const Parser::Token& token,
                 std::string message)
      : std::runtime_error(formatError(program, token, message)),
        program(program),
        token(token) {}

 private:
  static std::string formatError(const Parser::Program& program,
                                 const Parser::Token& token,
                                 std::string message);
};
