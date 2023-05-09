#include "transpile.hpp"

#include <iomanip>
#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>

const std::unordered_map<std::string, std::string> typeMap{
    {"integer", "int"},
};

std::string extractLiterals(const Parser::Token& token) {
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
  rec(token);
  return buf.str();
}

class ctranspiler {
 private:
  std::ostream& out;
  const Parser::Program& program;

  std::unordered_set<std::string> variables;

  void addVariable(const Parser::Token& id) {
    const auto literal = extractLiterals(id);
    if (variables.contains(literal)) {
      throw CTranspiler::TranspileError(
          program, id, "variable " + literal + " already declared");
    }

    variables.insert(literal);
  }

  void assertVariable(const Parser::Token& id) {
    const auto literal = extractLiterals(id);
    if (!variables.contains(literal)) {
      throw CTranspiler::TranspileError(
          program, id, "variable " + literal + " not declared");
    }
  }

 public:
  ctranspiler(std::ostream& out, const Parser::Program& program)
      : out(out), program(program) {}

  void walk(const Parser::Token& token) {
    if (token.children.empty()) {
      return;  // base case
    }

    if (token.type == "<prog>") {
      out << "#include <iostream>\n"
          << "\n"
          << "int main() {\n";

      walk(token.children.at(4).getToken());  // <dec-list>
      walk(token.children.at(6).getToken());  // <stat-list>

      out << "  return 0;\n"
          << "}\n";
      return;
    }

    if (token.type == "<dec-list>") {
      const auto type = token.children.at(2).getToken();  // <type>
      const auto dec = token.children.at(0).getToken();   // <dec>

      out << "  ";
      walk(type);
      out << " ";
      walk(dec);
      out << ";\n";

      return;
    }

    if (token.type == "<dec>") {
      const auto identifier = token.children.at(0).getToken();  // <identifier>
      const auto prime = token.children.at(1).getToken();       // <dec-prime>
      addVariable(identifier);

      out << extractLiterals(identifier);
      walk(prime);

      return;
    }

    if (token.type == "<dec-prime>") {
      const auto identifier = token.children.at(1).getToken();  // <identifier>
      const auto prime = token.children.at(2).getToken();       // <dec-prime>
      addVariable(identifier);

      out << ", " << extractLiterals(identifier);
      walk(prime);

      return;
    }

    if (token.type == "<type>") {
      const auto ourType = extractLiterals(token);
      const auto cxxType = typeMap.at(ourType);
      out << cxxType;

      return;
    }

    if (token.type == "<stat-list>" || token.type == "<stat-list-prime>") {
      const auto stat = token.children.at(0).getToken();   // <stat>
      const auto prime = token.children.at(1).getToken();  // <stat-list-prime>

      walk(stat);
      walk(prime);

      return;
    }

    if (token.type == "<stat>") {
      const auto child = token.children.at(0).getToken();  // <write> | <assign>

      out << "  ";
      walk(child);
      out << ";\n";

      return;
    }

    if (token.type == "<write>") {
      const auto prime = token.children.at(2).getToken();  // <write-prime>

      out << "std::cout";
      walk(prime);
      out << " << std::endl";

      return;
    }

    if (token.type == "<write-prime>") {
      if (token.children.size() == 3) {
        const auto string = token.children.at(0).getLiteral();  // σ
        const auto identifier =
            token.children.at(2).getToken();  // <identifier>
        out << " << " << string << " << " << extractLiterals(identifier);
      } else {
        const auto identifier =
            token.children.at(0).getToken();  // <identifier>
        out << " << " << extractLiterals(identifier);
      }

      return;
    }

    if (token.type == "<assign>") {
      const auto identifier = token.children.at(0).getToken();  // <identifier>
      const auto expression = token.children.at(2).getToken();  // <expr>

      assertVariable(identifier);

      out << extractLiterals(identifier) << " = ";
      walk(expression);

      return;
    }

    if (token.type == "<expr>") {
      const auto term = token.children.at(0).getToken();   // <term>
      const auto prime = token.children.at(1).getToken();  // <expr-prime>

      walk(term);
      walk(prime);

      return;
    }

    if (token.type == "<expr-prime>") {
      const auto op = token.children.at(0).getLiteral();
      const auto term = token.children.at(1).getToken();   // <term>
      const auto prime = token.children.at(2).getToken();  // <expr-prime>

      out << " " << op << " ";
      walk(term);
      walk(prime);

      return;
    }

    if (token.type == "<term>") {
      const auto factor = token.children.at(0).getToken();  // <factor>
      const auto prime = token.children.at(1).getToken();   // <term-prime>

      walk(factor);
      walk(prime);

      return;
    }

    if (token.type == "<term-prime>") {
      const auto op = token.children.at(0).getLiteral();
      const auto factor = token.children.at(1).getToken();  // <factor>
      const auto prime = token.children.at(2).getToken();   // <term-prime>

      out << " " << op << " ";
      walk(factor);
      walk(prime);

      return;
    }

    if (token.type == "<factor>") {
      if (token.children.size() == 3) {
        const auto expr = token.children.at(1).getToken();  // <expr>
        out << "(";
        walk(expr);
        out << ")";
      } else {
        const auto child = token.children.at(0).getToken();
        out << extractLiterals(child);
      }

      return;
    }

    throw std::runtime_error("Unknown token type: " + token.type);
  }
};

void CTranspiler::transpile(std::ostream& out, const Parser::Program& program) {
  ctranspiler trans(out, program);
  trans.walk(program);
}

std::string CTranspiler::TranspileError::formatError(
    const Parser::Program& program, const Parser::Token& token,
    std::string message) {
  std::stringstream ss;
  ss << "transpile error at token " << std::quoted(extractLiterals(token))
     << " " << token.type << ": " << message
     << formatLine(program.file, token.location());
  return ss.str();
}
