#pragma once

#include "parser.hpp"

class CTranspiler {
 public:
  CTranspiler() = default;
  static void transpile(std::ostream& out, const Parser::Token& program);
};
