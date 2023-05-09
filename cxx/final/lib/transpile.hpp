#pragma once

#include "parser.hpp"

class CTranspiler {
 public:
  class TranspileError;

  CTranspiler() = default;
  static void transpile(std::ostream& out, const Parser::Token& program);
};

// extractLiterals walks token and accumulates all literals into a string.
std::string extractLiterals(const Parser::Token& token);

class CTranspiler::TranspileError : public std::runtime_error {
 public:
  Parser::Token token;  // where the error occurred

  TranspileError(Parser::Token token, std::string message)
      : std::runtime_error(formatError(token, message)), token(token) {}

 private:
  static std::string formatError(Parser::Token token, std::string message) {
    std::stringstream ss;
    ss << "transpile error at token " << std::quoted(extractLiterals(token));
    ss << " " << token.type << ": " << message;
    return ss.str();
  }
};
