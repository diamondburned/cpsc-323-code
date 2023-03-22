/*----------------------------------------------------------------------
 *  Group Names :   Ethan Safai, Erik Nguyen, Diamond Dinh
 *  Assignment  :   No. 7 - Part 2
 *  Due Date    :   03/23/23
 *  Purpose: This program prompt the user for an expression, and then
 *  parse it based on the predictive parsing table defined in the
 *  handout assignment. It will print out the results, successful or not.
 *----------------------------------------------------------------------*/

#include <iostream>
#include <limits>
#include "expressionparser.h"

int main() {
    ExpressionParser parser = ExpressionParser();
    char inputContinue;

    do {
        std::cout << "Enter an expression with '$' at the end: ";
        std::string inputExpression;
        std::getline(std::cin, inputExpression);
        std::cout << std::endl;

        // Parses the expression.
        ExpressionParser::ParseResult result = parser.ParseExpression(inputExpression);
        if(result.Success()) {
            std::cout << "Successfully Parsed The Expression!" << std::endl;
            std::cout << result.LoggingMessage() << std::endl;
        }
        else {
            std::cout << result.ErrorMessage() << std::endl;
            std::cout << result.LoggingMessage() << std::endl;
        }

        // Continue y/n prompt
        std::cout << "CONTINUE(y/n)? ";
        std::cin >> inputContinue;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << std::endl;
    } while(tolower(inputContinue) == 'y');

    return 0;
}
