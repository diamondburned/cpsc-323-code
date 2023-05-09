#include "lexer.hpp"

#include <algorithm>
#include <cctype>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace {
struct lexingState {
  std::istream& in;
  std::vector<Lexer::Lexeme> line = {};
  Lexer::Lines lines = {};
  int64_t lineStart = 0;

  lexingState(std::istream& in_) : in(in_) {}

  int get() { return in.get(); }
  char getc() { return static_cast<char>(get()); }

  int peek() { return in.peek(); }

  bool drain(char c) {
    return drain([c](int x) { return c == x; });
  }

  bool drain(std::function<bool(int)> f) {
    bool found = false;
    while (f(peek())) {
      get();
      found = true;
    }
    return found;
  }

  std::string slurp(std::function<bool(int)> f) {
    std::string s;
    while (f(peek())) {
      const auto c = get();
      if (c == EOF) {
        throw std::runtime_error("unexpected EOF");
      }
      s += static_cast<char>(c);
    }
    return s;
  }

  void undo(size_t n) {
    for (size_t i = 0; i < n; i++) {
      in.unget();
    }
  }

  // aheadIs returns true if the next n characters are equal to the given
  // string.
  bool aheadIs(std::string_view s) {
    for (size_t i = 0; i < s.size(); i++) {
      if (peek() != s[i]) {
        undo(i);
        return false;
      }
      get();
    }
    undo(s.size());
    return true;
  }

  bool isWhitespace() { return isWhitespace(peek()); }
  bool isWhitespace(int c) { return std::isspace(c) || c == EOF; }

  void flushLine() {
    if (line.empty()) {
      return;
    }

    int64_t end = in.tellg();
    lines.push_back({lineStart, end, line});

    line.clear();
    lineStart = end;
  }
};

void trimSpace(std::string& str) {
  str.erase(0, str.find_first_not_of(" \t\r\n"));
  str.erase(str.find_last_not_of(" \t\r\n") + 1);
}

enum lexState {
  START,
  LINE,
  WORD,
  PUNCT,
  STRING,
  COMMENT,
  END,
};

// lexFunc is a function that consumes characters from the input stream and
// returns the next lexing state.
typedef std::function<lexState(lexingState&)> lexFunc;

// lexStateFuncs maps a lexing state to a lexing function.
static const std::unordered_map<lexState, lexFunc> lexStateFuncs({
    {START,
     [](lexingState& state) {
       if (state.peek() == '\n') {
         return LINE;
       }

       if (state.peek() == '"') {
         return STRING;
       }

       if (state.aheadIs("//")) {
         return COMMENT;
       }

       if (std::isalnum(state.peek())) {
         return WORD;
       }

       if (std::ispunct(state.peek())) {
         return PUNCT;
       }

       if (state.drain([](int c) { return std::isspace(c); })) {
         return START;
       }

       if (state.peek() == EOF) {
         state.flushLine();
         return END;
       }

       std::stringstream s;
       s << "unexpected character: " << state.peek() << " "
         << "(" << std::string(1, state.peek()) << ")" << std::endl;

       throw std::runtime_error(s.str());
     }},
    {LINE,
     [](lexingState& state) {
       state.drain('\n');
       state.flushLine();
       return START;
     }},
    {WORD,
     [](lexingState& state) {
       int64_t start = state.in.tellg();
       auto word =
           state.slurp([](char c) { return std::isalnum(c) || c == '.'; });

       int64_t end = state.in.tellg();
       state.line.push_back(
           Lexer::Lexeme{start, end, Lexer::Lexeme::WORD, word});
       return START;
     }},
    {PUNCT,
     [](lexingState& state) {
       int64_t start = state.in.tellg();
       char terminator = state.getc();
       state.line.push_back({start, start + 1, Lexer::Lexeme::PUNCT,
                             std::string(1, terminator)});
       return START;
     }},
    {STRING,
     [](lexingState& state) {
       int64_t start = state.in.tellg();
       state.get();  // consume the opening quote
       auto str = state.slurp([](char c) { return c != '"'; });
       state.get();  // consume the closing quote
       int64_t end = state.in.tellg();
       state.line.push_back({start, end, Lexer::Lexeme::STRING, str});
       return START;
     }},
    {COMMENT,
     [](lexingState& state) {
       int64_t start = state.in.tellg();
       std::string comment;
       while (true) {
         auto line = state.slurp([](char c) { return c != '\n'; });
         if (line.starts_with("//")) {
           line = line.substr(2);
           trimSpace(line);
           comment += line;

           state.get();  // consume the newline
           continue;
         }

         // Line does not start with "//", so we check to see if it ends
         // with "//". If it does, then we're done with comments.
         if (line.ends_with("//")) {
           line = line.substr(0, line.size() - 2);
           trimSpace(line);
           comment += line;

           state.get();  // consume the newline
           break;
         }

         // If it doesn't, then it's actually part of a new statement, so
         // we must unget the line. Add 1 to undo the newline.
         state.undo(line.size() + 1);
         break;
       }

       int64_t end = state.in.tellg();
       state.line.push_back(
           Lexer::Lexeme{start, end, Lexer::Lexeme::COMMENT, comment});
       return START;
     }},
});
}  // namespace

Lexer::Lines Lexer::lex(std::istream& in) {
  lexingState lexing(in);
  lexState lex = START;
  while (lex != END) {
    lex = lexStateFuncs.at(lex)(lexing);
  }
  return lexing.lines;
}

int Lexer::Location::length() const { return end - start; }

bool Lexer::Location::isEOF() const { return start == -1 && end == -1; }

bool Lexer::Location::includes(const Location& other) const {
  return start <= other.start && other.end <= end;
}

Lexer::Location Lexer::Location::merge(const Location& other) const {
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

bool Lexer::Lexeme::isEOF() const { return loc.start == -1 && loc.end == -1; }

bool Lexer::Lexeme::includes(const Lexeme& other) const {
  return loc.includes(other.loc);
}

Lexer::Lexeme Lexer::Lexeme::slice(int64_t start, int64_t end) const {
  return Lexeme(loc.start + start, loc.start + end, type,
                value.substr(start, end - start));
}

std::vector<Lexer::Lexeme> Lexer::Lexeme::separate() const {
  std::vector<Lexeme> tokens;
  for (size_t i = 0; i < value.size(); i++) {
    tokens.push_back(slice(i, i + 1));
  }
  return tokens;
}

void Lexer::Lexeme::print(std::ostream& out) const {
  switch (type) {
    case WORD:
    case PUNCT:
      out << value;
      break;
    case STRING:
      out << std::quoted(value);
      break;
    case COMMENT:
      out << "// " << value << " //";
      break;
  }
}

Lexer::Location Lexer::Line::relativeLocation(const Location& loc) const {
  int col = 0;
  for (const auto& t : *this) {
    if (t.loc.includes(loc)) {
      return {col, col + t.loc.length()};
    }
    col += t.loc.length() + 1;
  }
  return {-1, -1};
}

void Lexer::Line::print(std::ostream& out) const {
  size_t i = 0;
  for (const auto& token : *this) {
    if (i > 0 && token.value != ".") {
      out << " ";
    }
    out << token;
    i++;
  }
}

size_t Lexer::Lines::containingLine(const Lexer::Location& loc) const {
  for (size_t i = 0; i < size(); i++) {
    if (at(i).loc.includes(loc)) {
      return i;
    }
  }
  return -1;
}

Lexer::Lexeme Lexer::Lines::findCompleteLexeme(
    const Lexer::Lexeme lexeme) const {
  for (const auto& line : *this) {
    for (const auto& token : line) {
      if (token.includes(lexeme)) {
        return token;
      }
    }
  }
  return lexeme;
}

Lexer::Lines Lexer::Lines::removeComments() const {
  Lexer::Lines result;
  result.reserve(size());

  for (const auto& line : *this) {
    std::vector<Lexer::Lexeme> tokens;
    tokens.reserve(line.size());
    std::copy_if(line.begin(), line.end(), std::back_inserter(tokens),
                 [](const Lexer::Lexeme& t) {
                   return t.type != Lexer::Lexeme::COMMENT;
                 });
    if (!tokens.empty()) {
      result.push_back({line.loc, tokens});
    }
  }

  return result;
}

std::vector<Lexer::Lexeme> Lexer::Lines::flatten() const {
  std::vector<Lexer::Lexeme> result;
  for (const auto& line : *this) {
    std::copy(line.begin(), line.end(), std::back_inserter(result));
  }
  return result;
}

void Lexer::Lines::print(std::ostream& out) const {
  for (size_t i = 0; i < size(); i++) {
    out << at(i);
    if (i < size() - 1) {
      out << "\n";
    }
  }
}
