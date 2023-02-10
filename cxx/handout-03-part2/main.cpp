/*----------------------------------------------------------------------
 *  Group Names :   Ethan Safai, Erik Nguyen, Diamond Dinh
 *  Assignment  :   No. 3 - Part 2
 *  Due Date    :   02/16/23
 *  Purpose: This program reads a word and determine if it is accepted
 *  by the grammar.
 *----------------------------------------------------------------------*/

#include <iostream>
#include <string>
#include <limits>

//Transition table defined by the CFG given in the assignment worksheet.
const int TRANSITION_TABLE[4][3] = {{0,1,2},
                                    {2,1,3},
                                    {0,3,3},
                                    {1,3,2}};

//VerifyWord function declaration
bool VerifyWord(const std::string& word);


int main() {
    char inputContinue;
    do {
        //Prompt for a postfix expression.
        std::cout << "Enter a Word with $ at the end: ";
        std::string inputExpression;
        std::getline(std::cin, inputExpression);

        //Calls function to verify the word according to the grammar.
        try {
            if(VerifyWord(inputExpression)) {
                std::cout << inputExpression << " is accepted." << std::endl;
            }
            else {
                std::cout << inputExpression << " is rejected." << std::endl;
            }
        }
        catch (std::invalid_argument &exception) {
            std::cout << exception.what() << std::endl;
        }

        //Continue y/n prompt
        std::cout << "\nCONTINUE(y/n)? ";
        std::cin >> inputContinue;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    } while(tolower(inputContinue) == 'y');

    return 0;
}

/**
 * Determines if a given word is accepted by the grammar defined
 * by the global constant array TRANSITION_TABLE.
 * @param word Word that will be tested.
 * @return If the word is accepted or rejected.
 */
bool VerifyWord(const std::string& word)
{
    if(word.at(word.length() - 1) != '$') {
        throw std::invalid_argument("Word must end with '$'");
    }

    int col = 0;
    int state = 0;

    for(int index = 0; index < word.length() - 1; index++) {
        switch(word[index]) {
            case 'a':
                col = 0;
                break;
            case 'b':
                col = 1;
                break;
            case 'c':
                col = 2;
                break;
            default:
                //Word contains invalid letter.
                return false;
        }
        state = TRANSITION_TABLE[state][col];
    }

    //These are the two ending-states that are accepted by the grammar.
    return state == 1 || state == 2;
}
