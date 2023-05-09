#include "error.hpp"

#include <iostream>

std::string formatLine(const Lexer::Lines& lines, Lexer::Lexeme lexeme) {
  return formatLine(lines, lexeme.loc);
}

std::string formatLine(const Lexer::Lines& lines, Lexer::Location loc) {
  const int linenum = lines.containingLine(loc);
  if (linenum == -1) {
    return "";
  }

  const auto& line = lines[linenum];
  const auto lineLocation = line.relativeLocation(loc);

  std::stringstream ss;
  ss << "\n"
     << "    | " << line << "\n"
     << "    | " << std::string(lineLocation.start, ' ')
     << std::string(lineLocation.length(), '^');

  return ss.str();
}
