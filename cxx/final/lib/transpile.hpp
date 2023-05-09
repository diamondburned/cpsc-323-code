#pragma once

#include <iostream>

#include "parser.hpp"

class CTranspiler {
 public:
  class TranspileError;

  CTranspiler() = default;
  static void transpile(std::ostream& out, const Parser::Program& program);
};

// extractLiterals walks token and accumulates all literals into a string.
std::string extractLiterals(const Parser::Token& token);

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
                                 std::string message) {
    std::stringstream ss;
    ss << "transpile error at token " << std::quoted(extractLiterals(token))
       << " " << token.type << ": " << message
       << formatLine(program.file, token.location());
    return ss.str();
  }
};
