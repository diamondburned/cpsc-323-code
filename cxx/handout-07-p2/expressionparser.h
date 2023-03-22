
#ifndef PREDICTIVEPARSER_EXPRESSIONPARSER_H
#define PREDICTIVEPARSER_EXPRESSIONPARSER_H

#include <string>
#include <stack>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <iostream>
#include <sstream>

class ExpressionParser
{
public:
    enum Tokens {
        // All terminals tokens must have at least 1 character mapped to each. (Except Lambda)
        TERMINAL_CHAR,
        TERMINAL_PLUS,
        TERMINAL_MINUS,
        TERMINAL_MULT,
        TERMINAL_DIV,
        TERMINAL_RIGHT_PARENTHESIS,
        TERMINAL_LEFT_PARENTHESIS,
        TERMINAL_EQUALS,
        TERMINAL_DOLLAR,
        TERMINAL_LAMBDA, // Unused for actual input expression

        NON_TERMINAL_S,
        NON_TERMINAL_W,
        NON_TERMINAL_E,
        NON_TERMINAL_Q,
        NON_TERMINAL_T,
        NON_TERMINAL_R,
        NON_TERMINAL_F,

        INVALID_TOKEN // Used if a character in the input string is invalid.
    };

    /**
     * Class that represents the result of a parse. If the parse is unsuccessful, there will be an
     * error message present. Conversely, there will be no error message if the parse is successful.
     */
    class ParseResult
    {
    public:
        explicit ParseResult(bool success, const std::stringstream& error, const std::stringstream& log) {
            this->isSuccessful = success;
            this->errorMessage = error.str();
            this->loggingMessage = log.str();
        }
        bool Success() const {
            return isSuccessful;
        }
        const std::string& ErrorMessage() const {
            return errorMessage;
        }
        const std::string& LoggingMessage() const {
            return loggingMessage;
        }
    private:
        bool isSuccessful = false;
        std::string errorMessage;
        std::string loggingMessage;
    };


    /**
     * Instantiates a new ExpressionParser object.
     */
    ExpressionParser() {
        ConstructParseTable();
        ConstructTokenDefinition();
    }

    /**
     * Parses the given string expression based on the parse table.
     * @param expression Expression that will be parsed.
     * @return ParseResult
     */
    ParseResult ParseExpression(const std::string& expression) const;

protected:
    /**
     * Gets the token type based on the character from the input string.
     * This information is set from the token definition table.
     * @param character Specific character from the input string.
     * @return Token type.
     */
    Tokens GetTokenTypeFromCharacter(char character) const;

    /**
     * Creates a printable string based on the token contents of the stack.
     * Every Non-Terminal entry in the stack will be displayed based on how it
     * is set in the switch statement in the method below. This method is used
     * for the log.
     *
     * If there are multiple characters defined within a token type, it will
     * display the first entry of the character set.
     *
     * @param tokens Stack of tokens.
     * @return Stringify display of the stack.
     */
    std::string GetPrintableStackStringDisplay(std::stack<Tokens> tokens) const;

private:
    // This is the starting non-terminal.
    const Tokens STARTING_NON_TERMINAL = NON_TERMINAL_S;
    std::unordered_map<Tokens, std::unordered_map<Tokens, std::vector<Tokens>>> parseTable;
    std::unordered_map<Tokens, std::unordered_set<char>> tokenTable;

    /**
     * This is where the predictive parsing table data is set.
     * The order for the array matters and should be in order with the predictive parsing table.
     */
    void ConstructParseTable() {
        //State S row
        this->parseTable[NON_TERMINAL_S][TERMINAL_CHAR] = {TERMINAL_CHAR, NON_TERMINAL_W};

        //State W row
        this->parseTable[NON_TERMINAL_W][TERMINAL_EQUALS] = {TERMINAL_EQUALS, NON_TERMINAL_E};

        // State E row
        this->parseTable[NON_TERMINAL_E][TERMINAL_CHAR] = {NON_TERMINAL_T, NON_TERMINAL_Q};
        this->parseTable[NON_TERMINAL_E][TERMINAL_LEFT_PARENTHESIS] = {NON_TERMINAL_T, NON_TERMINAL_Q};

        // State Q row
        this->parseTable[NON_TERMINAL_Q][TERMINAL_PLUS] = {TERMINAL_PLUS, NON_TERMINAL_T, NON_TERMINAL_Q};
        this->parseTable[NON_TERMINAL_Q][TERMINAL_MINUS] = {TERMINAL_MINUS, NON_TERMINAL_T, NON_TERMINAL_Q};
        this->parseTable[NON_TERMINAL_Q][TERMINAL_RIGHT_PARENTHESIS] = {TERMINAL_LAMBDA};
        this->parseTable[NON_TERMINAL_Q][TERMINAL_DOLLAR] = {TERMINAL_LAMBDA};

        // State T row
        this->parseTable[NON_TERMINAL_T][TERMINAL_CHAR] = {NON_TERMINAL_F, NON_TERMINAL_R};
        this->parseTable[NON_TERMINAL_T][TERMINAL_LEFT_PARENTHESIS] = {NON_TERMINAL_F, NON_TERMINAL_R};

        // State R row
        this->parseTable[NON_TERMINAL_R][TERMINAL_PLUS] = {TERMINAL_LAMBDA};
        this->parseTable[NON_TERMINAL_R][TERMINAL_MINUS] = {TERMINAL_LAMBDA};
        this->parseTable[NON_TERMINAL_R][TERMINAL_MULT] = {TERMINAL_MULT, NON_TERMINAL_F, NON_TERMINAL_R};
        this->parseTable[NON_TERMINAL_R][TERMINAL_DIV] = {TERMINAL_DIV, NON_TERMINAL_F, NON_TERMINAL_R};
        this->parseTable[NON_TERMINAL_R][TERMINAL_RIGHT_PARENTHESIS] = {TERMINAL_LAMBDA};
        this->parseTable[NON_TERMINAL_R][TERMINAL_DOLLAR] = {TERMINAL_LAMBDA};

        // State F row
        this->parseTable[NON_TERMINAL_F][TERMINAL_CHAR] = {TERMINAL_CHAR};
        this->parseTable[NON_TERMINAL_F][TERMINAL_LEFT_PARENTHESIS] = {TERMINAL_LEFT_PARENTHESIS,
                                                                       NON_TERMINAL_E, TERMINAL_RIGHT_PARENTHESIS};
    }

    /**
     * Maps characters to their associated terminal tokens.
     * Each terminal with the exception of lambda must have at least 1 character mapped to it.
     */
    void ConstructTokenDefinition() {
        this->tokenTable[TERMINAL_CHAR] = {'a'};
        this->tokenTable[TERMINAL_EQUALS] = {'='};
        this->tokenTable[TERMINAL_PLUS] = {'+'};
        this->tokenTable[TERMINAL_MINUS] = {'-'};
        this->tokenTable[TERMINAL_MULT] = {'*'};
        this->tokenTable[TERMINAL_DIV] = {'/'};
        this->tokenTable[TERMINAL_LEFT_PARENTHESIS] = {'('};
        this->tokenTable[TERMINAL_RIGHT_PARENTHESIS] = {')'};
        this->tokenTable[TERMINAL_DOLLAR] = {'$'};
    }
};


/**
* Parses the given string expression based on the parse table.
* @param expression Expression that will be parsed.
* @return ParseResult
*/
ExpressionParser::ParseResult ExpressionParser::ParseExpression(const std::string& expression) const {
    std::stringstream loggingMessage;

    // Ensures that $ is at the end of the expression
    if(expression.at(expression.length() - 1) != '$') {
        loggingMessage << "Ran into error." << std::endl;
        return ParseResult(false, std::stringstream("Missing '$' at end\n"), loggingMessage);
    }

    // Adds initials to the stack
    std::stack<Tokens> parseStack;
    parseStack.push(TERMINAL_DOLLAR);
    parseStack.push(this->STARTING_NON_TERMINAL);

    int index = 0;
    while(!parseStack.empty() && index < expression.length()) {
        Tokens popped = parseStack.top();
        parseStack.pop();

        Tokens readToken = GetTokenTypeFromCharacter(expression[index]);
        if(readToken == popped) {
            // Matched, so advance to next character
            loggingMessage << "Match \'" << expression[index] << "\'  |  "
                           << GetPrintableStackStringDisplay(parseStack) << std::endl;
            index++;
            continue;
        }

        std::vector<Tokens> tableEntry;
        try {
            tableEntry = this->parseTable.at(popped).at(readToken);
        }
        catch(const std::exception& exception) {
            // Error, meaning we hit an invalid character or an empty entry in the table.
            std::stringstream errorMessage;
            errorMessage << "Error at index " << index << " | ";
            int frontLength = errorMessage.str().length();
            errorMessage << expression << std::endl;

            errorMessage << std::string(frontLength + index, ' ') << "^" << std::endl;
            loggingMessage << "Ran into error." << std::endl;

            ParseResult result = ParseResult(false, errorMessage, loggingMessage);
            return result;
        }

        // Adds to stack based on the entry in the table.
        for (auto iterator = tableEntry.rbegin(); iterator != tableEntry.rend(); ++iterator ) {
            if(*iterator == TERMINAL_LAMBDA) {
                // Ignore lambda terminals
                continue;
            }
            parseStack.push(*iterator);
        }
    }

    return ParseResult(true, std::stringstream(), loggingMessage);
}

/**
* Gets the token type based on the character from the input string.
* This information is set from the token definition table.
* @param character Specific character from the input string.
* @return Token type.
*/
ExpressionParser::Tokens ExpressionParser::GetTokenTypeFromCharacter(char character) const {
    for(const auto& tokenPair : this->tokenTable) {
        if(tokenPair.second.find(character) != tokenPair.second.end()) {
            return tokenPair.first;
        }
    }
    return INVALID_TOKEN;
}

/**
* Creates a printable string based on the token contents of the stack.
* Every Non-Terminal entry in the stack will be displayed based on how it
* is set in the switch statement in the method below. This method is used
* for the log.
*
* If there are multiple characters defined within a token type, it will
* display the first entry of the character set.
*
* @param tokens Stack of tokens.
* @return Stringify display of the stack.
*/
std::string ExpressionParser::GetPrintableStackStringDisplay(std::stack<Tokens> tokens) const {
    std::string display;
    while(!tokens.empty()) {
        if(this->tokenTable.find(tokens.top()) != this->tokenTable.end()) {
            // Adds first character of the character set to the display
            char character = *this->tokenTable.at(tokens.top()).begin();
            display = character + display;
        }
        else {
            // Displays Non-Terminals
            switch(tokens.top())
            {
                case NON_TERMINAL_E:
                    display = "E" + display;
                    break;
                case NON_TERMINAL_T:
                    display = "T" + display;
                    break;
                case NON_TERMINAL_R:
                    display = "R" + display;
                    break;
                case NON_TERMINAL_Q:
                    display = "Q" + display;
                    break;
                case NON_TERMINAL_F:
                    display = "F" + display;
                    break;
                case NON_TERMINAL_S:
                    display = "S" + display;
                    break;
                case NON_TERMINAL_W:
                    display = "W" + display;
                    break;
            }
        }
        tokens.pop();
    }
    return display;
}

#endif //PREDICTIVEPARSER_EXPRESSIONPARSER_H
