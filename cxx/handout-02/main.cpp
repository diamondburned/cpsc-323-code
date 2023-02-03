//------------------------------------------------------------------------------
// Group names: Ethan Safai, Erik Nguyen, Diamond Dinh
// Assignment : No. 2
// Due date   : 02/09/23
//
// Purpose: this program reads an expression in postfix form with potential
// variables, reads those variables, and then evaluates the expression and 
// displays its result.
//------------------------------------------------------------------------------
#include <limits>
#include <ios>
#include "postfixparser.h"

int main() {
  int result; // result of the postfix expression
  char choice; // whether the user wants to exit the program
  std::string expr; // the postfix expression entered by the user
  
  // read in and evaluate postfix expressions from the user until they wish to
  // quit
  while (true) {
    // read in and validate expression
    std::cout << "Enter a postfix expression with a $ at the end:\n";
    std::getline(std::cin, expr);
    if (expr.length() < 2 || expr.back() != '$') {
      throw std::runtime_error("Invalid expression");
    }

    // evaluate expression and display result
    PostfixParser parser(expr);
    result = parser.evalExpr();

    std::cout << "Expression's value is " << result << "\n\n";

    // check if user wishes to continue
    std::cout << "CONTINUE(y/n)? ";
    std::cin >> choice;

    if (choice != 'y') {
      break;
    }

    std::cout << '\n';
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
  }

  return 0;
}