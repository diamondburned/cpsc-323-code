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
bool lexemeMatches(Lexer::Lexeme lexeme, std::string expects) {
  switch (lexeme.type) {
    case Lexer::Lexeme::STRING:
      return expects == SIGMA;
    default:
      return expects == lexeme.value;
  }
}

struct sentinel {
  std::string type;
  Lexer::Lexeme lexeme;
  Parser::Token* node;
};

Parser::Program Parser::parse(const Lexer::Lines& file) const {
  if (file.empty()) {
    throw Parser::SyntaxError(file, Lexer::Lexeme(), "empty file");
  }

  std::stack<Lexer::Lexeme> lexemeStack;
  push_vector(lexemeStack, file.flatten());

  Parser::Program root(file);

  // Adds initials to the stack
  std::stack<sentinel> parseStack;
  parseStack.push(sentinel{"$", Lexer::Lexeme(), nullptr});
  parseStack.push(sentinel{startingGrammar.first, lexemeStack.top(), &root});

  while (!parseStack.empty() && !lexemeStack.empty()) {
    auto lexeme = lexemeStack.top();
    if (lexeme.type == Lexer::Lexeme::WORD && lexeme.value.length() > 1) {
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
            file, lexeme, "unexpected terminal token, expecting " + type);
      }
      node->add(lexeme);
      lexemeStack.pop();
      continue;
    }

    std::vector<std::string> tableEntry;
    try {
      std::string value = lexeme.value;
      if (lexeme.type == Lexer::Lexeme::Type::STRING) {
        // All string literals are represented as a sigma in the table.
        // Mask the value as a sigma before looking up in the table.
        value = SIGMA;
      }
      tableEntry = parsingTable.at(type).at(value);
    } catch (const std::exception& exception) {
      if (errorEntryTable.contains(type)) {
        const auto errors = errorEntryTable.at(type);
        if (errors.contains(lexeme.value)) {
          throw Parser::SyntaxError(file, lexeme, errors.at(lexeme.value));
        }
        if (errors.contains("?")) {
          throw Parser::SyntaxError(file, lexeme, errors.at("?"));
        }
      }
      throw Parser::SyntaxError(file, lexeme,
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

std::string Parser::SyntaxError::formatError(const Lexer::Lines& file,
                                             Lexer::Lexeme lexeme,
                                             std::string message) {
  std::stringstream ss;
  ss << "syntax error near word " << std::quoted(lexeme.value) << ": "
     << message << formatLine(file, lexeme);
  return ss.str();
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

Lexer::Location Parser::Token::location() const {
  Lexer::Location loc;
  for (const auto& child : children) {
    switch (child.type) {
      case Value::Type::TOKEN:
        loc = loc.merge(child.getToken().location());
        break;
      case Value::Type::LITERAL:
        loc = loc.merge(child.getLiteral().loc);
        break;
      default:
        break;
    }
  }
  return loc;
}

std::string Parser::Token::extractLiterals() const {
  std::stringstream buf;
  std::function<void(const Parser::Token&)> rec;
  rec = [&buf, &rec](const Parser::Token& token) {
    for (const auto& child : token.children) {
      switch (child.type) {
        case Parser::Token::Value::Type::LITERAL:
          buf << child.getLiteral();
          break;
        case Parser::Token::Value::Type::TOKEN:
          rec(child.getToken());
          break;
        default:
          break;
      }
    }
  };
  rec(*this);
  return buf.str();
}
