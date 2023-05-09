#include "transpile.hpp"

#include <iomanip>
#include <string>
#include <unordered_map>
#include <unordered_set>

const std::unordered_map<std::string, std::string> typeMap{
    {"integer", "int"},
};

// extractLiterals walks token and accumulates all literals into a string.
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

struct ctranspiler {
  std::unordered_set<std::string> variables;

  void walkProgram(std::ostream& out, const Parser::Token& token) {
    if (token.children.empty()) {
      return;  // base case
    }

    if (token.type == "<prog>") {
      out << "#include <iostream>\n"
          << "\n"
          << "int main() {\n";

      walkProgram(out, token.children.at(4).getToken());  // <dec-list>
      walkProgram(out, token.children.at(6).getToken());  // <stat-list>

      out << "  return 0;\n"
          << "}\n";
      return;
    }

    if (token.type == "<dec-list>") {
      const auto type = token.children.at(2).getToken();  // <type>
      const auto dec = token.children.at(0).getToken();   // <dec>

      out << "  ";
      walkProgram(out, type);
      out << " ";
      walkProgram(out, dec);
      out << ";\n";

      return;
    }

    if (token.type == "<dec>") {
      const auto identifier = token.children.at(0).getToken();  // <identifier>
      const auto prime = token.children.at(1).getToken();       // <dec-prime>

      const auto identifierLiteral = extractLiterals(identifier);
      if (variables.contains(identifierLiteral)) {
        throw std::runtime_error("variable " + identifierLiteral +
                                 " already declared");
      }

      variables.insert(identifierLiteral);

      out << identifierLiteral;
      walkProgram(out, prime);

      return;
    }

    if (token.type == "<dec-prime>") {
      const auto identifier = token.children.at(1).getToken();  // <identifier>
      const auto prime = token.children.at(2).getToken();       // <dec-prime>

      out << ", " << extractLiterals(identifier);
      walkProgram(out, prime);

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

      walkProgram(out, stat);
      walkProgram(out, prime);

      return;
    }

    if (token.type == "<stat>") {
      const auto child = token.children.at(0).getToken();  // <write> | <assign>

      out << "  ";
      walkProgram(out, child);
      out << ";\n";

      return;
    }

    if (token.type == "<write>") {
      const auto prime = token.children.at(2).getToken();  // <write-prime>

      out << "std::cout";
      walkProgram(out, prime);
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

      const auto identifierLiteral = extractLiterals(identifier);
      if (!variables.contains(identifierLiteral)) {
        throw std::runtime_error("variable " + identifierLiteral +
                                 " not declared");
      }

      out << identifierLiteral << " = ";
      walkProgram(out, expression);

      return;
    }

    if (token.type == "<expr>") {
      const auto term = token.children.at(0).getToken();   // <term>
      const auto prime = token.children.at(1).getToken();  // <expr-prime>

      walkProgram(out, term);
      walkProgram(out, prime);

      return;
    }

    if (token.type == "<expr-prime>") {
      const auto op = token.children.at(0).getLiteral();
      const auto term = token.children.at(1).getToken();   // <term>
      const auto prime = token.children.at(2).getToken();  // <expr-prime>

      out << " " << op << " ";
      walkProgram(out, term);
      walkProgram(out, prime);

      return;
    }

    if (token.type == "<term>") {
      const auto factor = token.children.at(0).getToken();  // <factor>
      const auto prime = token.children.at(1).getToken();   // <term-prime>

      walkProgram(out, factor);
      walkProgram(out, prime);

      return;
    }

    if (token.type == "<term-prime>") {
      const auto op = token.children.at(0).getLiteral();
      const auto factor = token.children.at(1).getToken();  // <factor>
      const auto prime = token.children.at(2).getToken();   // <term-prime>

      out << " " << op << " ";
      walkProgram(out, factor);
      walkProgram(out, prime);

      return;
    }

    if (token.type == "<factor>") {
      if (token.children.size() == 3) {
        const auto expr = token.children.at(1).getToken();  // <expr>
        out << "(";
        walkProgram(out, expr);
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

void CTranspiler::transpile(std::ostream& out, const Parser::Token& program) {
  ctranspiler trans;
  trans.walkProgram(out, program);
}
