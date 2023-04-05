#include <istream>
#include <string_view>
#include <vector>

// Lexer defines a lexer, which parses an input stream into a list of lexemes.
namespace Lexer {
struct Token {
  enum Type {
    WORD,
    PUNCT,
    COMMENT,
  };

  int64_t start;
  int64_t end;
  Type type;
  std::string value;
};

struct Line {
  int64_t start;
  int64_t end;
  std::vector<Token> tokens;
};

std::vector<Line> lex(std::istream& in);

void print(std::ostream& out, const std::vector<Line>& lines);
void print(std::ostream& out, const Line& line);
void print(std::ostream& out, const Token& token);
}  // namespace Lexer
