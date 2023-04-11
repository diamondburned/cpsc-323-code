#include <iomanip>
#include <istream>
#include <string>
#include <string_view>
#include <vector>

// Lexer defines a lexer, which parses an input stream into a list of lexemes.
namespace Lexer {
struct Location {
  int64_t start;
  int64_t end;
};

struct Token {
  enum Type {
    WORD,
    PUNCT,
    STRING,
    COMMENT,
  };

  Location loc;
  Type type;
  std::string value;

  Token(int64_t start, int64_t end, Type type, std::string value)
      : loc{start, end}, type(type), value(value) {}

  Token(Location loc, Type type, std::string value)
      : loc(loc), type(type), value(value) {}

  friend std::ostream& operator<<(std::ostream& out, const Token& t) {
    switch (t.type) {
      case WORD:
      case PUNCT:
        out << t.value;
        break;
      case STRING:
        out << std::quoted(t.value);
        break;
      case COMMENT:
        out << "// " << t.value << " //";
        break;
    }
    return out;
  };
};

struct Line : std::vector<Token> {
  Location loc;

  Line(int64_t start, int64_t end, std::vector<Token> tokens)
      : std::vector<Token>(tokens), loc({start, end}) {}

  Line(Location loc, std::vector<Token> tokens)
      : std::vector<Token>(tokens), loc(loc) {}

  friend std::ostream& operator<<(std::ostream& out, const Line& line) {
    size_t i = 0;
    for (const auto& token : line) {
      if (i > 0 && token.value != ".") {
        out << " ";
      }
      out << token;
      i++;
    }
    return out;
  };
};

struct Lines : std::vector<Line> {
  Lines() = default;
  Lines(std::vector<Line> lines) : std::vector<Line>(lines) {}

  friend std::ostream& operator<<(std::ostream& out, const Lines& ls) {
    for (size_t i = 0; i < ls.size(); i++) {
      out << ls[i];
      if (i < ls.size() - 1) {
        out << "\n";
      }
    }
    return out;
  };
};

Lines lex(std::istream& in);
Lines removeComments(const Lines& lines);

enum printFlags {
  NONE = 0,
  IGNORE_COMMENTS = 1 << 0,
};
}  // namespace Lexer
