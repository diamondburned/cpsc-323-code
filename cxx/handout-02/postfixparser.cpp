#include "postfixparser.h"

bool PostfixParser::isInt(const std::string& s) {
  // check if each character is a digit
  for (const char& c : s) {
    if (!std::isdigit(c)) {
      return false;
    }
  }
  return true;
}

void PostfixParser::getTokens(const std::string& expr) {
  bool done;
  std::string token;

  int i = 0, n = expr.length();
  done = false;
  // split the expression into a vector of individual tokens
  while (!done) {
    // ignore leading whitespace
    token.clear();
    while (i < n && std::isspace(expr[i])) {
      ++i;
    }
    // read characters, appending to the current token and then pushing it back
    // to the vector once the end of expression or another space is encountered
    while (i < n) {
      if (expr[i] == '$') {
        // done, push back last token read and exit loops
        if (token.length() > 0) {
          this->tokens.push_back(token);
        }
        done = true;
        break;
      }
      if (!std::isspace(expr[i])) {
        token += expr[i];
        ++i;
      } else {
        this->tokens.push_back(token);
        ++i;
        break;
      }
    }
  }
}

void PostfixParser::getVars() {
  std::string value;

  // iterate over tokens
  for (const std::string& tok : this->tokens) {
    // if the current token is an identifier, read in its value and add it to
    // the vars map
    if (std::isalpha(tok[0])) {
      std::cout << "Enter the value of " << tok << ": ";
      std::getline(std::cin, value);
      if (!isInt(value)) {
        throw std::runtime_error("Expected integer, received {" + value + '}');
      }
      this->vars[tok] = std::stoi(value);
    }
  }
}

int PostfixParser::evalBinaryExpr(int l, char op, int r) const {
  switch (op) {
  case '+': return l + r;
  case '-': return l - r;
  case '*': return l * r;
  case '/': return l / r;
  default: throw std::runtime_error("Expected operator, received {" + op + '}');
  }
}

int PostfixParser::evalExpr() const {
  int l, r;
  std::stack<int> stack;

  // iterate over tokens
  for (const std::string& token : this->tokens) {
    if (this->vars.find(token) != this->vars.end()) { // token is identifier
      stack.push(this->vars.at(token));
    } else if (isInt(token)) { // token is digit
      stack.push(stoi(token));
    } else { // token is operator
      // validate operator and current state of the stack
      if (token != "+" && token != "-" && token != "*" && token != "/") {
        throw std::runtime_error("Expected operator, received {" + token + '}');
      }
      if (stack.size() < 2) {
        throw std::runtime_error("Expected two operands for operator " + token);
      }
      // evaluate expression and push result onto stack
      r = stack.top();
      stack.pop();
      l = stack.top();
      stack.pop();
      stack.push(evalBinaryExpr(l, token[0], r));
    }
  }

  // should be exactly one element left in the stack or else the expression is
  // invalid
  if (stack.size() != 1) {
    throw std::runtime_error("Invalid expression");
  }

  return stack.top();
}