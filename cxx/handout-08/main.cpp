/*----------------------------------------------------------------------
 *  Group Names :   Ethan Safai, Erik Nguyen, Diamond Dinh
 *  Assignment  :   No. 8
 *  Due Date    :   04/11/23
 *  Purpose: This program will parse a given string expression based on
 *  the given LR parsing table from the assignment. It will print out
 *  the steps and results of the parse.
 *----------------------------------------------------------------------*/

#include "lr_parser.h"

int main() {
    LRExpressionParser parser = LRExpressionParser("lrtable.csv", "rules.csv");

    std::string expression;
    std::cout << "Expression: ";
    std::getline(std::cin, expression);
    std::cout << std::endl;

    parser.ParseExpression(expression);

    return 0;
}
