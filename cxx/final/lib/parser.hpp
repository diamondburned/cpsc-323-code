#pragma once

#include <functional>
#include <list>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "error.hpp"
#include "grammar.hpp"
#include "lexer.hpp"

class Parser {
 public:
  class SyntaxError;
  class Token;
  class Program;

  /**
   * Instantiates a new ProgramParser object.
   * @param fileLoc Text File Location of the grammar
   * @param errorInfoLoc Text File for the error state messages.
   */
  Parser(const Grammar& grammar);

  /**
   * Compiles the given text file program.
   * @param inputFileLoc Text File of the program.
   */
  Program parse(const Lexer::Lines& file) const;

  /**
   * Loads the error entry message file into the parser. This specifies what
   * type of error messages are printed dependent on the invalid entry during
   * the parse.
   */
  void loadErrorEntries(std::istream& errorEntries);
  void loadErrorEntries(std::string errorEntriesPath);

 private:
  std::unordered_map<std::string, std::unordered_map<std::string, std::string>>
      errorEntryTable;

  std::unordered_map<std::string,
                     std::map<std::string, std::vector<std::string>>>
      parsingTable;

  std::pair<std::string, std::vector<std::string>> startingGrammar;

  std::unordered_set<std::string> reserved;
  std::unordered_set<std::string> terminals;
};

class Parser::SyntaxError : public std::runtime_error {
 public:
  const Lexer::Lines& file;  // the entire program
  Lexer::Lexeme lexeme;      // where the error occurred

  SyntaxError(const Lexer::Lines& file, Lexer::Lexeme lexeme,
              std::string message)
      : std::runtime_error(
            formatError(file, file.findCompleteLexeme(lexeme), message)),
        file(file),
        lexeme(file.findCompleteLexeme(lexeme)) {}

 private:
  static std::string formatError(const Lexer::Lines& file, Lexer::Lexeme lexeme,
                                 std::string message);
};

class Parser::Token {
 public:
  class Value;  // I can't define this inline :(
  friend class Parser;

  std::string type;  // <prog>, <identifier>, <dec-list>, ...
  std::vector<Value> children;

  Token() : type("$") {}
  Token(const std::string& type) : type(type) {}

  friend std::ostream& operator<<(std::ostream& out, const Token& token) {
    token.print(out);
    return out;
  };

  // location returns the location covering the entire token.
  Lexer::Location location() const;

  // extractLiterals walks token and accumulates all literals into a string.
  std::string extractLiterals() const;

 private:
  bool isEOF() const { return type == "$"; }
  void print(std::ostream& out, int level = 0) const;

  template <class T>
  Parser::Token::Value* add(const T& value);
};

class Parser::Program : public Parser::Token {
 public:
  friend class Parser;

  const Lexer::Lines& file;

 private:
  Program(const Lexer::Lines& file) : Token(), file(file) {}
};

class Parser::Token::Value {
 public:
  enum Type {
    NONE,
    TOKEN,
    LITERAL,
  };

  Type type;

  Value() : type(NONE) {}

  Value(Token token) : type(TOKEN), token(std::make_unique<Token>(token)) {}

  Value(Lexer::Lexeme literal)
      : type(LITERAL), literal(std::make_unique<Lexer::Lexeme>(literal)) {}

  Value(const Value& other) : type(other.type) {
    switch (type) {
      case NONE:
        break;
      case TOKEN:
        token = std::make_unique<Token>(*other.token);
        break;
      case LITERAL:
        literal = std::make_unique<Lexer::Lexeme>(*other.literal);
        break;
    }
  }

  Token* getToken() {
    assertType(TOKEN);
    return token.get();
  }

  Token getToken() const {
    assertType(TOKEN);
    return *token;
  }

  Lexer::Lexeme* getLiteral() {
    assertType(LITERAL);
    return literal.get();
  }

  Lexer::Lexeme getLiteral() const {
    assertType(LITERAL);
    return *literal;
  }

 private:
  static std::string typeAsString(Type t) {
    switch (t) {
      case TOKEN:
        return "TOKEN";
      case LITERAL:
        return "LITERAL";
      default:
        return "NONE";
    }
  }

  void assertType(Type t) const {
    if (type != t) {
      std::stringstream ss;
      ss << "expected type " << typeAsString(t) << ", got "
         << typeAsString(type);
      throw std::logic_error(ss.str());
    }
  }

  // I'm balls deep in this shit. I hate C++ so much.
  // You can hardly pick a worse language for this university.
  std::unique_ptr<Token> token;
  std::unique_ptr<Lexer::Lexeme> literal;
};
