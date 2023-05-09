#pragma once

#include <iomanip>
#include <istream>
#include <string>
#include <string_view>
#include <vector>

// Lexer defines a lexer, which parses an input stream into a list of lexemes.
struct Lexer {
  struct Location;
  struct Lexeme;
  struct Line;
  struct Lines;

  static Lines lex(std::istream& in);
};

struct Lexer::Location {
  int64_t start;
  int64_t end;

  Location() : start(-1), end(-1) {}
  Location(int64_t start, int64_t end) : start(start), end(end) {}

  bool operator==(const Location& other) const {
    return start == other.start && end == other.end;
  }

  int length() const;
  bool isEOF() const;
  bool includes(const Location& other) const;
  Location merge(const Location& other) const;
};

struct Lexer::Lexeme {
  enum Type {
    WORD,
    PUNCT,
    STRING,
    COMMENT,
  };

  Location loc;
  Type type;
  std::string value;

  Lexeme() : type(WORD), value("") {}  // EOF token

  Lexeme(int64_t start, int64_t end, Type type, std::string value)
      : loc{start, end}, type(type), value(value) {}

  Lexeme(Location loc, Type type, std::string value)
      : loc(loc), type(type), value(value) {}

  bool isEOF() const;
  bool includes(const Lexeme& other) const;

  // slice returns a new token that is a slice of the current token.
  Lexeme slice(int64_t start, int64_t end) const;

  // separate returns a list of tokens of one character each.
  std::vector<Lexeme> separate() const;

  bool operator==(const Lexeme& other) const {
    return loc == other.loc && type == other.type && value == other.value;
  }

  friend std::ostream& operator<<(std::ostream& out, const Lexeme& t) {
    t.print(out);
    return out;
  };

 private:
  void print(std::ostream& out) const;
};

struct Lexer::Line : std::vector<Lexeme> {
  Location loc;

  Line(int64_t start, int64_t end, std::vector<Lexeme> tokens)
      : std::vector<Lexeme>(tokens), loc({start, end}) {}

  Line(Location loc, std::vector<Lexeme> tokens)
      : std::vector<Lexeme>(tokens), loc(loc) {}

  friend std::ostream& operator<<(std::ostream& out, const Line& line) {
    line.print(out);
    return out;
  };

  // relativeLocation returns the location of the given token relative to the
  // start of the line. If the token is not found, it returns {-1, -1}.
  Location relativeLocation(const Location& loc) const;

 private:
  void print(std::ostream& out) const;
};

struct Lexer::Lines : std::vector<Line> {
  Lines() = default;
  Lines(std::vector<Line> lines) : std::vector<Line>(lines) {}

  friend std::ostream& operator<<(std::ostream& out, const Lines& ls) {
    ls.print(out);
    return out;
  };

  // containingLine returns the line number that contains the given token or
  // -1 if the token is not found.
  size_t containingLine(const Location& loc) const;

  // findCompleteLexeme returns the lexeme that covers the given token or the
  // given lexeme if it it's not found.
  Lexer::Lexeme findCompleteLexeme(const Lexer::Lexeme lexeme) const;

  // removeComments returns a new list of lines with all comments removed.
  Lines removeComments() const;

  // flatten returns a new list of tokens with all lines concatenated.
  std::vector<Lexeme> flatten() const;

 private:
  void print(std::ostream& out) const;
};
