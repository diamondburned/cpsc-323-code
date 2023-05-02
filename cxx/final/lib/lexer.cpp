#include "lexer.hpp"

#include <algorithm>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace Lexer {

namespace {
struct lexingState {
  std::istream& in;
  std::vector<Token> line = {};
  Lines lines = {};
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
       auto word = state.slurp([](char c) { return std::isalnum(c); });

       int64_t end = state.in.tellg();
       state.line.push_back(Token{start, end, Token::WORD, word});
       return START;
     }},
    {PUNCT,
     [](lexingState& state) {
       int64_t start = state.in.tellg();
       char terminator = state.getc();
       state.line.push_back(
           {start, start + 1, Token::PUNCT, std::string(1, terminator)});
       return START;
     }},
    {STRING,
     [](lexingState& state) {
       int64_t start = state.in.tellg();
       state.get();  // consume the opening quote
       auto str = state.slurp([](char c) { return c != '"'; });
       state.get();  // consume the closing quote
       int64_t end = state.in.tellg();
       state.line.push_back({start, end, Token::STRING, str});
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
       state.line.push_back(Token{start, end, Token::COMMENT, comment});
       return START;
     }},
});
}  // namespace

Lines lex(std::istream& in) {
  lexingState lexing(in);
  lexState lex = START;
  while (lex != END) {
    lex = lexStateFuncs.at(lex)(lexing);
  }
  return lexing.lines;
}

Lines removeComments(const Lines& lines) {
  Lines result;
  result.reserve(lines.size());

  for (const auto& line : lines) {
    std::vector<Token> tokens;
    tokens.reserve(line.size());
    std::copy_if(line.begin(), line.end(), std::back_inserter(tokens),
                 [](const Token& t) { return t.type != Token::COMMENT; });
    if (!tokens.empty()) {
      result.push_back({line.loc, tokens});
    }
  }

  return result;
}
}  // namespace Lexer
