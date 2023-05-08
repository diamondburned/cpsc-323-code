#pragma once

#include <functional>
#include <list>
#include <memory>
#include <string>
#include <vector>

#include "grammar.hpp"
#include "lexer.hpp"

class Parser {
 public:
  class SyntaxError;
  class Token;

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
  Token parse(const std::vector<Lexer::Token>& tokens) const;

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
  Lexer::Token lexeme;  // where the error occurred

 public:
  SyntaxError(Lexer::Token lexeme, std::string message)
      : std::runtime_error(formatError(lexeme, message)), lexeme(lexeme) {}

  static std::string formatError(Lexer::Token lexeme, std::string message) {
    std::stringstream ss;
    ss << "syntax error at word " << std::quoted(lexeme.value) << ": "
       << message;
    return ss.str();
  }
};

class Parser::Token {
 public:
  class Value;  // I can't define this inline :(
  friend class Parser;

  std::string type;  // <prog>, <identifier>, <dec-list>, ...
  std::vector<Value> children;

  Token() : type("$") {}
  Token(const std::string& type) : type(type) {}

  // walk does a depth-first traversal of the tree, calling the callback on each
  // node.
  void walk(std::function<void(const Value&)> callback) const;

  friend std::ostream& operator<<(std::ostream& out, const Token& token) {
    token.print(out);
    return out;
  };

 private:
  bool isEOF() const { return type == "$"; }
  void print(std::ostream& out, int level = 0) const;

  template <class T>
  Parser::Token::Value* add(const T& value);
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

  Value(Lexer::Token literal)
      : type(LITERAL), literal(std::make_unique<Lexer::Token>(literal)) {}

  Value(const Value& other) : type(other.type) {
    switch (type) {
      case NONE:
        break;
      case TOKEN:
        token = std::make_unique<Token>(*other.token);
        break;
      case LITERAL:
        literal = std::make_unique<Lexer::Token>(*other.literal);
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

  Lexer::Token* getLiteral() {
    assertType(LITERAL);
    return literal.get();
  }

  Lexer::Token getLiteral() const {
    assertType(LITERAL);
    return *literal;
  }

 private:
  void assertType(Type t) const {
    if (type != t) {
      throw std::logic_error("value type mismatch");
    }
  }

  // I'm balls deep in this shit. I hate C++ so much.
  // You can hardly pick a worse language for this university.
  std::unique_ptr<Token> token;
  std::unique_ptr<Lexer::Token> literal;
};
