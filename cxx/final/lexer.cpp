#include "lexer.hpp"

#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace Lexer {
enum lexState {
  START,
  LINE,
  WORD,
  PUNCT,
  COMMENT,
  END,
};

struct lexingState {
  std::istream& in;
  std::vector<Line> lines = {};
  std::vector<Token> line = {};
  int64_t stmtStart = 0;

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
        break;
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
};

void trimSpace(std::string& str) {
  str.erase(0, str.find_first_not_of(" \t\r\n"));
  str.erase(str.find_last_not_of(" \t\r\n") + 1);
}

static const std::unordered_map<lexState, std::function<lexState(lexingState&)>>
    lexStateFuncs({
        {START,
         [](lexingState& state) {
           if (state.peek() == '\n') {
             return LINE;
           }

           if (state.peek() == '/') {
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

           int64_t end = state.in.tellg();
           state.lines.push_back({state.stmtStart, end, state.line});

           state.line.clear();
           state.stmtStart = end;
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
             // we must unget the line.
             state.undo(line.size());
             break;
           }

           int64_t end = state.in.tellg();
           state.line.push_back(Token{start, end, Token::COMMENT, comment});
           return START;
         }},
    });

std::vector<Line> lex(std::istream& in) {
  lexingState lexing(in);
  lexState lex = START;
  while (lex != END) {
    lex = lexStateFuncs.at(lex)(lexing);
  }
  return lexing.lines;
}

void print(std::ostream& out, const std::vector<Line>& lines) {
  for (const auto& line : lines) {
    print(out, line);
    out << "\n";
  }
}

void print(std::ostream& out, const Line& line) {
  size_t i = 0;
  for (const auto& token : line.tokens) {
    if (token.type == Token::COMMENT) {
      continue;
    }

    if (i > 0 && token.value != ".") {
      out << " ";
    }
    print(out, token);

    i++;
  }
}

void print(std::ostream& out, const Token& token) { out << token.value; }
}  // namespace Lexer
