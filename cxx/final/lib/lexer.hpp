#pragma once

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

  Location() : start(-1), end(-1) {}
  Location(int64_t start, int64_t end) : start(start), end(end) {}

  bool operator==(const Location& other) const {
    return start == other.start && end == other.end;
  }

  bool isEOF() const { return start == -1 && end == -1; }

  bool includes(const Location& other) const {
    return start <= other.start && other.end <= end;
  }

  Location merge(const Location& other) const {
    if (other.isEOF()) {
      return *this;
    }
    Location merged(*this);
    if (merged.start == -1 || other.start < merged.start) {
      merged.start = other.start;
    }
    if (merged.end == -1 || other.end > merged.end) {
      merged.end = other.end;
    }
    return merged;
  }

  int length() const { return end - start; }
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

  Token() : type(WORD), value("") {}  // EOF token

  Token(int64_t start, int64_t end, Type type, std::string value)
      : loc{start, end}, type(type), value(value) {}

  Token(Location loc, Type type, std::string value)
      : loc(loc), type(type), value(value) {}

  bool isEOF() const { return loc.start == -1 && loc.end == -1; }

  // slice returns a new token that is a slice of the current token.
  Token slice(int64_t start, int64_t end) const {
    return Token(loc.start + start, loc.start + end, type,
                 value.substr(start, end - start));
  }

  // separate returns a list of tokens of one character each.
  std::vector<Token> separate() const {
    std::vector<Token> tokens;
    for (size_t i = 0; i < value.size(); i++) {
      tokens.push_back(slice(i, i + 1));
    }
    return tokens;
  }

  // join joins the current token with the other token.
  void join(const Token& other) {
    loc.end = other.loc.end;
    value += other.value;
  }

  // isLeftOf returns true if the current token is directly left of the other
  // token.
  bool isLeftOf(const Token& other) const { return loc.end == other.loc.start; }

  bool includes(const Token& other) const { return loc.includes(other.loc); }

  bool operator==(const Token& other) const {
    return loc == other.loc && type == other.type && value == other.value;
  }

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

  // relativeLocation returns the location of the given token relative to the
  // start of the line. If the token is not found, it returns {-1, -1}.
  Location relativeLocation(const Location& loc) const {
    int col = 0;
    for (const auto& t : *this) {
      if (t.loc.includes(loc)) {
        return {col, col + t.loc.length()};
      }
      col += t.loc.length() + 1;
    }
    return {-1, -1};
  }
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

  // containingLine returns the line number that contains the given token or -1
  // if the token is not found.
  size_t containingLine(const Location& loc) const {
    for (size_t i = 0; i < size(); i++) {
      if (at(i).loc.includes(loc)) {
        return i;
      }
    }
    return -1;
  }
};

Lines lex(std::istream& in);
Lines removeComments(const Lines& lines);
std::vector<Token> flatten(const Lines& lines);

enum printFlags {
  NONE = 0,
  IGNORE_COMMENTS = 1 << 0,
};
}  // namespace Lexer
