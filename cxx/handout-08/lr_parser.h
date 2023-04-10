
#ifndef LRPARSERASSIGNMENT_LR_PARSER_H
#define LRPARSERASSIGNMENT_LR_PARSER_H

#include <iostream>
#include <vector>
#include <stack>
#include <unordered_map>
#include <string>
#include <sstream>
#include <fstream>

class LRExpressionParser
{
public:
    /**
     * Instantiates a new LRExpressionParser object.
     * @param tableFileLoc CSV file location with the LR Parsing Table
     * @param rulesFileLoc CSV file location with the Rules Table
     */
    explicit LRExpressionParser(const std::string& tableFileLoc, const std::string& rulesFileLoc) {
        ConstructTable(tableFileLoc);
        ConstructRules(rulesFileLoc);
    }

    /**
     * Parses an expression based on the tables.
     * The results of the parse will be printed.
     * @param expression Expression that will be parsed.
     */
    void ParseExpression(const std::string& expression) {
        if(expression.at(expression.length() - 1) != '$') {
            std::cout << "Expression must contain '$' as the last character." << std::endl;
            std::cout << "INVALID EXPRESSION" << std::endl;
            return;
        }

        //Declares and adds 0 to the stack
        std::stack<std::string> expressionStack;
        expressionStack.emplace("0");

        int readIndex = 0;
        std::string readTerminal;
        bool readNext = true;

        while(!expressionStack.empty()) {
            std::cout << "================================" << std::endl;
            std::cout << "Stack: ";
            PrintStack(expressionStack);

            //Pops from the stack
            std::string popped = expressionStack.top();
            expressionStack.pop();
            std::cout << "Popped: " << popped << std::endl;


            //If the previous iterations calls for reading the next character in the expression
            if(readNext) {
                readTerminal = std::string(1, expression.at(readIndex));
                readIndex++;
                std::cout << "Read: " << readTerminal << std::endl;
            }

            //Get Entry based on [popped, readTerminal]
            std::string entry;
            try {
                entry = this->parseTable.at(popped).at(readTerminal); //Goto [popped, readTerminal]

                std::cout << "[" << popped << ", " << readTerminal << "]" << " = " << entry << std::endl;
            } catch(std::exception& e) {
                std::cout << "[" << popped << ", " << readTerminal << "]" << " is empty!" << std::endl;
                std::cout << "\nINVALID EXPRESSION" << std::endl;
                return;
            }


            if(std::tolower(entry.at(0)) == 's') {
                //Sn Entry
                std::cout << "Push: " << popped << ", " << readTerminal << ", " << entry.substr(1) << std::endl;
                expressionStack.push(popped);               //k
                expressionStack.push(readTerminal);         //t
                expressionStack.push(entry.substr(1));  //n
                readNext = true;
            }
            else if(std::tolower(entry.at(0)) == 'r') {
                //Rn Entry
                std::cout << "Push: " << popped << std::endl;
                //Push k back to the stack
                expressionStack.push(popped);


                std::pair<std::string, std::string> rule = this->rules.at(entry.substr(1));
                std::cout << "Rule #" << entry.substr(1) << ": " << rule.first << " -> " << rule.second << std::endl;

                //Pop twice length of right side
                std::cout << "Pop " << rule.second.length() * 2 << " times" << std::endl;
                for(int count = 0; count < rule.second.length() * 2; ++count) {
                    expressionStack.pop();
                }

                std::cout << "Stack: ";
                PrintStack(expressionStack);


                //Goto NonTerminal Entry
                std::string nextState = expressionStack.top();  //m
                expressionStack.pop();
                std::cout << "Popped: " << nextState << std::endl;

                //Go to Entry at [nextState, leftSide Of Rule]
                try {
                    std::string numberEntry = this->parseTable.at(nextState).at(rule.first); //Goto [m, leftSide Of Rule]
                    std::cout << "[" << nextState << ", " << rule.first << "]" << " = " << numberEntry << std::endl;

                    //Pushes to the stack
                    std::cout << "Push: " << nextState << ", " << rule.first << ", " << numberEntry << std::endl;
                    expressionStack.push(nextState);
                    expressionStack.push(rule.first);
                    expressionStack.push(numberEntry);

                    readNext = false;
                } catch(std::exception& e) {
                    std::cout << "[" << popped << ", " << readTerminal << "]" << " is empty!" << std::endl;
                    std::cout << "\nINVALID EXPRESSION" << std::endl;
                    return;
                }
            }
            else if(std::tolower(entry.at(0)) == 'a') {
                //ACC Entry
                std::cout << "\nEXPRESSION ACCEPTED" << std::endl;
                return;
            }
        }
    }

    /**
     * Prints the contents of a string stack.
     * @param stack Stack that will be printed.
     */
    static void PrintStack(std::stack<std::string> stack) {
        std::string output;
        while(!stack.empty()) {
            output = stack.top() + " " + output;
            stack.pop();
        }
        std::cout << output << std::endl;
    }

private:
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> parseTable;
    std::unordered_map<std::string, std::pair<std::string, std::string>> rules;

    /**
     * Constructs the parsing table based on the given CSV file.
     * @param fileLoc CSV file location with the LR Parsing Table
     */
    void ConstructTable(const std::string& fileLoc) {
        std::ifstream csvFile(fileLoc);
        this->parseTable = std::unordered_map<std::string, std::unordered_map<std::string, std::string>>();

        // Gets column header
        std::string line;
        std::getline(csvFile, line);
        std::vector<std::string> colHeader = ParseCSVRow(line);

        while(std::getline(csvFile, line)) {
            std::vector<std::string> entries = ParseCSVRow(line);
            if(entries.empty() || entries[0].empty()) {
                continue;
            }

            std::string rowHeading = entries[0];
            for(int index = 1; index < entries.size(); ++index) {
                if(entries[index].empty()) {
                    continue;
                }
                this->parseTable[rowHeading][colHeader[index]] = entries[index];
            }
        }
    }

    /**
     * Constructs the rules table based on the given CSV file.
     * @param fileLoc CSV file location with the rules table
     */
    void ConstructRules(const std::string& fileLoc) {
        std::ifstream csvFile(fileLoc);
        this->rules = std::unordered_map<std::string, std::pair<std::string, std::string>>();

        std::string line;
        int ruleNumber = 1; //Rule numbers start at 1
        while(std::getline(csvFile, line)) {
            std::vector<std::string> entries = ParseCSVRow(line);
            if(entries.size() < 2 || entries[0].empty() || entries[1].empty()) {
                continue;
            }
            this->rules[std::to_string(ruleNumber)] = std::pair<std::string, std::string>(entries[0], entries[1]);
            ruleNumber++;
        }
    }

    /**
     * Helper function that parses a row from a CSV file.
     * @param line String row of a CSV file.
     * @return Vector of string entries from the given row.
     */
    static std::vector<std::string> ParseCSVRow(const std::string& line) {
        std::vector<std::string> entries;
        std::stringstream stream(line);

        std::string value;
        while (std::getline(stream, value, ',')) {
            entries.push_back(value);
        }
        return entries;
    }
};

#endif //LRPARSERASSIGNMENT_LR_PARSER_H
