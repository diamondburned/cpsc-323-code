#include "parser.hpp"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <optional>
#include <stack>

Parser::Parser(const Grammar& grammar) {
  parsingTable = grammar.constructPredictiveParsingTable();
  startingGrammar = grammar.getStartingGrammar();

  for (const auto& terminal : grammar.getTerminals()) {
    terminals.insert(terminal);
    if (terminal.length() > 1 || terminal == LAMBDA) {
      reserved.insert(terminal);
    }
  }
}

template <typename T>
void push_vector(std::stack<T>& stack, const std::vector<T>& values) {
  for (auto it = values.rbegin(); it != values.rend(); it++) {
    stack.push(*it);
  }
}

// lexemeMatches returns true if the lexeme matches the expected value.
bool lexemeMatches(Lexer::Token lexeme, std::string expects) {
  switch (lexeme.type) {
    case Lexer::Token::STRING:
      return expects == SIGMA;
    default:
      return expects == lexeme.value;
  }
}

struct sentinel {
  std::string type;
  Lexer::Token lexeme;
  Parser::Token* node;
};

Parser::Token Parser::parse(const std::vector<Lexer::Token>& lexemes) const {
  if (lexemes.empty()) {
    return Parser::Token();
  }

  std::stack<Lexer::Token> lexemeStack;
  push_vector(lexemeStack, lexemes);

  // std::vector<Lexer::Token> lexemes;
  // for (const auto& token : inputLexemes) {
  //   // Split words that are not terminals into individual characters for the
  //   // parser to ingest.
  //   if (token.type == Lexer::Token::WORD && !reserved.contains(token.value))
  //   {
  //     const auto separated = token.separate();
  //     std::copy(separated.begin(), separated.end(),
  //               std::back_inserter(lexemes));
  //     continue;
  //   }
  //   lexemes.push_back(token);
  // }

  Parser::Token root = Parser::Token();

  // Adds initials to the stack
  std::stack<sentinel> parseStack;
  parseStack.push(sentinel{"$", Lexer::Token(), nullptr});
  parseStack.push(sentinel{startingGrammar.first, lexemes.at(0), &root});

  while (!parseStack.empty() && !lexemeStack.empty()) {
    auto lexeme = lexemeStack.top();
    if (lexeme.type == Lexer::Token::WORD && lexeme.value.length() > 1) {
      if (!reserved.contains(lexeme.value)) {
        // Replace the lexeme with the split lexemes.
        lexemeStack.pop();
        push_vector(lexemeStack, lexeme.separate());
        continue;
      }
    }

    auto top = parseStack.top();
    parseStack.pop();

    Parser::Token* node = top.node;
    const std::string type = top.type;

    if (terminals.contains(type)) {
      if (!lexemeMatches(lexeme, type)) {
        throw Parser::SyntaxError(
            lexeme, "unexpected terminal token, expecting " + type);
      }
      node->add(lexeme);
      lexemeStack.pop();
      continue;
    }

    std::vector<std::string> tableEntry;
    try {
      std::string value = lexeme.value;
      if (lexeme.type == Lexer::Token::Type::STRING) {
        // All string literals are represented as a sigma in the table.
        // Mask the value as a sigma before looking up in the table.
        value = SIGMA;
      }
      tableEntry = parsingTable.at(type).at(value);
    } catch (const std::exception& exception) {
      const auto errors = errorEntryTable.at(type);
      if (errors.contains(lexeme.value)) {
        throw Parser::SyntaxError(lexeme, errors.at(lexeme.value));
      }
      if (errors.contains("?")) {
        throw Parser::SyntaxError(lexeme, errors.at("?"));
      }
      throw Parser::SyntaxError(lexeme,
                                "unexpected non-terminal, expecting " + type);
    }

    if (node->isEOF()) {
      // Probably root not initialized.
      node->type = type;
    } else {
      // Append a new node.
      node = node->add(Parser::Token(type))->getToken();
    }

    // Adds to stack based on the entry in the table.
    for (auto it = tableEntry.rbegin(); it != tableEntry.rend(); it++) {
      // Ignore lambda terminals
      if (*it != LAMBDA) {
        parseStack.push(sentinel{*it, lexeme, node});
      }
    }
  }

  if (root.isEOF()) {
    throw std::logic_error("unexpected root node is EOF");
  }

  return root;
}

void Parser::loadErrorEntries(std::string path) {
  std::ifstream f(path);
  Parser::loadErrorEntries(f);
}

void Parser::loadErrorEntries(std::istream& errorEntries) {
  std::string line;
  while (std::getline(errorEntries, line)) {
    std::vector<std::string> entries;
    std::stringstream stream(line);

    std::string value;
    while (std::getline(stream, value, ' ')) {
      entries.push_back(value);
    }
    if (entries.size() <= 3 || entries[2] != "|") {
      continue;
    }

    // Update errorEntryTable Mapping
    std::stringstream message;
    for (size_t index = 3; index < entries.size(); ++index) {
      message << entries[index] << " ";
    }

    errorEntryTable[entries[0]][entries[1]] = message.str();
  }
}

void indent(std::ostream& out, int level) {
  for (int i = 0; i < level; i++) {
    out << "  ";
  }
}

void Parser::Token::walk(std::function<void(const Value&)> callback) const {
  for (const auto& child : children) {
    callback(child);
    if (child.type == Parser::Token::Value::Type::TOKEN) {
      child.getToken().walk(callback);
    }
  }
}

void Parser::Token::print(std::ostream& out, int level) const {
  for (size_t i = 0; i < children.size(); i++) {
    const auto& child = children[i];
    indent(out, level);
    switch (child.type) {
      case Parser::Token::Value::Type::LITERAL:
        out << std::quoted(child.getLiteral().value) << std::endl;
        break;
      case Parser::Token::Value::Type::TOKEN:
        out << child.getToken().type << std::endl;
        child.getToken().print(out, level + 1);
        break;
      default:
        throw std::logic_error("unexpected token type");
    }
  }
}

template <class T>
Parser::Token::Value* Parser::Token::add(const T& value) {
  return &children.emplace_back(value);
}
