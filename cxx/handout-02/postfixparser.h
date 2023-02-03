#ifndef POSTFIXPARSER_H_
#define POSTFIXPARSER_H_

#include <iostream>
#include <stack>
#include <string>
#include <unordered_map>
#include <vector>

/**
 * Parses and evaluates a postfix expression
*/
class PostfixParser {
private:
  // a list of the tokens encountered in the expression
  std::vector<std::string> tokens;
  // a list of variables encountered in the expression and their corresponding
  // values
  std::unordered_map<std::string, int> vars;

  /**
   * Returns true if the string holds an integer, otherwise false
  */
  static inline bool isInt(const std::string& s);

  /**
   * Splits the expression into a vector of tokens until '$' is encountered,
   * ignoring all whitespace
  */
  void getTokens(const std::string& expr);

  /**
   * Reads in the values of identifiers found in the expression from stdin
  */
  void getVars();

  /**
   * Returns the result of a binary expression
  */
  int evalBinaryExpr(int l, char op, int r) const;

public:
  /**
   * @param expr the postfix expression to be evaluated
  */
  PostfixParser(std::string& expr) {
    getTokens(expr);
    getVars();
  }

  /**
   * Evaluates and returns the result of the postfix expression
  */
  int evalExpr() const;
};

#endif // POSTFIXPARSER_H_