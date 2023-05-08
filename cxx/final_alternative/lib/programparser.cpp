#include "programparser.hpp"
#include <iostream>
#include <fstream>

ProgramParser::ProgramParser(const std::string &grammarFileLoc, const std::string& errorEntriesLoc)
{
    GrammarParser grammar = GrammarParser(grammarFileLoc);
    this->parsingTable = grammar.ConstructPredictiveParsingTable();
    this->startingGrammar = grammar.GetStartingGrammar();

    // Keeps track of terminals and reserve words from the grammar
    for(const auto& terminal : grammar.GetTerminals()) {
        if(terminal.length() > 1) {
            this->reserveWords.insert(terminal);
        }
        this->terminals.insert(terminal);
    }

    if(!errorEntriesLoc.empty()) {
        PrepareErrorEntriesFromFile(errorEntriesLoc);
    }
}

void ProgramParser::LoadCompilerTranslationFile(const std::string &translationLoc)
{
    TranslationMap mapping;
    std::ifstream textFile(translationLoc);
    if(!textFile || !textFile.is_open()) {
        std::cout << "Failed to open " << translationLoc << std::endl;
        return;
    }

    std::string line;
    while(std::getline(textFile, line)) {
        std::vector<std::string> entries;
        std::stringstream stream(line);

        std::string value;
        while (std::getline(stream, value, ' ')) {
            entries.push_back(value);
        }
        if (entries.size() <= 2) {
            continue;
        }

        //Update Translation Mapping
        if (entries.size() >= 3 && entries[1] == "->") {
            int pipeIndex = -1;
            for (int index = 2; index < entries.size(); ++index) {
                if(entries[index] == "|") {
                    pipeIndex = index;
                    break;
                }
            }
            if(pipeIndex == -1) {
                // This means no translation exists for the grammar rule
                continue;
            }

            auto newEntries = std::vector<std::string>(entries.begin() + 2, entries.begin() + pipeIndex);
            auto translation = std::vector<std::string>(entries.begin() + pipeIndex + 1, entries.end());
            auto pair = std::pair<std::string, std::vector<std::string>>(entries[0], newEntries);

            mapping[pair] = translation;
            continue;
        }

        textFile.close();
        throw std::invalid_argument(line);
    }
    textFile.close();
    this->compilerTranslationMap = mapping;
}

bool ProgramParser::CompileProgram(const std::string& inputFileLoc) const
{
    // Ensures that translation mapping is set
    if(this->compilerTranslationMap.empty()) {
        std::cout << "Load a Compiler Grammar Translation File first!" << std::endl;
        return false;
    }

    // Opens Unprepared Program Text File
    std::ifstream unpreparedTextFile(inputFileLoc);
    if(!unpreparedTextFile || !unpreparedTextFile.is_open()) {
        std::cout << "Failed to open " << inputFileLoc << std::endl;
        return false;
    }
    // Lexer Prepares the Text File and Converts to Tokens
    auto lines = Lexer::removeComments(Lexer::lex(unpreparedTextFile));
    unpreparedTextFile.close();
    std::ofstream stage1(inputFileLoc + ".1.tmp");
    stage1 << lines << std::endl;
    stage1.close();


    // Opens Prepared Program Text File
    std::vector<std::string> tokens;
    std::ifstream preparedTextFile(inputFileLoc + ".1.tmp");
    if(!preparedTextFile || !preparedTextFile.is_open()) {
        std::cout << "Failed to open " << inputFileLoc + ".1.tmp" << std::endl;
        return false;
    }
    // Creates vector of tokens from the prepared tokens file
    std::string line;
    while(std::getline(preparedTextFile, line)) {
        std::stringstream stream(line);
        std::string value;
        while (std::getline(stream, value, ' ')) {
            tokens.push_back(value);
        }
    }
    preparedTextFile.close();


    // Constructs root of syntax tree
    std::shared_ptr<SyntaxNode> root = std::make_shared<SyntaxNode>();
    root->rule.first = "$";

    // Adds initials to the stack
    std::stack<std::shared_ptr<SyntaxNode>> parseStack;
    parseStack.push(root);
    parseStack.push(root->CreateChild(this->startingGrammar));

    int index = 0;
    while(!parseStack.empty() && index < tokens.size()) {
        std::shared_ptr<SyntaxNode> currentNode = parseStack.top();
        std::string popped = currentNode->rule.first;
        parseStack.pop();
        std::string readToken = tokens[index];

        // Splits Identifiers, Strings, etc. into multiple terminals
        if (readToken.length() > 1 && this->reserveWords.find(readToken) == this->reserveWords.end()) {
            tokens.erase(tokens.begin() + index);
            for (auto iterator = readToken.rbegin(); iterator != readToken.rend(); ++iterator) {
                tokens.insert(tokens.begin() + index, std::string(1, *iterator));
            }
            readToken = tokens.at(index);
        }


        if (this->terminals.find(popped) != this->terminals.end()) {
            if(readToken == popped) {
                // Matched, so advance to next character
                // This also indicates a leaf node, therefore set the second part of the grammar.
                currentNode->rule.second.clear();
                currentNode->rule.second.push_back(readToken);

                index++;
                continue;
            }
            else {
                // Program excepts a certain reserve word, got something else instead.
                std::cout << "Program contains Syntax Error." << std::endl;
                std::cout << popped << " was expected" << std::endl;
                return false;
            }
        }

        std::vector<std::string> tableEntry;
        try {
            tableEntry = this->parsingTable.at(popped).at(readToken);
            // Determined the current rule
            currentNode->rule.second = tableEntry;
        }
        catch(const std::exception& exception) {
            // Error, meaning we hit an invalid character or an empty entry in the table.
            // This means we read an incorrect entry from the parsing table.
            // Prints out Error Message
            std::cout << "Program contains Syntax Error." << std::endl;
            if(this->errorEntryTable.find(popped) != this->errorEntryTable.end()) {
                if(this->errorEntryTable.at(popped).find(readToken) != this->errorEntryTable.at(popped).end()) {
                    std::cout << this->errorEntryTable.at(popped).at(readToken) << std::endl;
                    return false;
                }
                else if(this->errorEntryTable.at(popped).find("?") != this->errorEntryTable.at(popped).end()) {
                    // ? entry is an all encompassing entry, meaning all errors related to the popped value is printed here.
                    std::cout << this->errorEntryTable.at(popped).at("?") << std::endl;
                    return false;
                }
            }
            std::cout << "Empty Entry at: " << "[" << popped << ", " << readToken << "]" << std::endl;
            std::cout << "Error at token " << index << std::endl;
            return false;
        }

        // Adds to stack based on the entry in the table.
        for (auto iterator = tableEntry.rbegin(); iterator != tableEntry.rend(); ++iterator ) {
            if(*iterator == RESERVE_WORD_LAMBDA) {
                // Ignore lambda terminals
                continue;
            }

            // Adds child node to the current
            parseStack.push(currentNode->CreateChild(GrammarParser::GrammarEntry(*iterator, tableEntry)));
        }
    }

    // If the parse was successful, print out the compiled code.
    std::string compiledCodeResult = BuildCode(root);

    std::cout << compiledCodeResult << std::endl;
    std::ofstream outputFile("output.txt");
    outputFile << compiledCodeResult << std::endl;
    outputFile.close();

    return true;
}

void ProgramParser::PrepareErrorEntriesFromFile(const std::string &errorEntriesLoc)
{
    std::ifstream textFile(errorEntriesLoc);
    if(!textFile || !textFile.is_open()) {
        std::cout << "Failed to open " << errorEntriesLoc << std::endl;
        return;
    }

    std::string line;
    while(std::getline(textFile, line)) {
        std::vector<std::string> entries;
        std::stringstream stream(line);

        std::string value;
        while (std::getline(stream, value, ' ')) {
            entries.push_back(value);
        }
        if (entries.size() <= 3 || entries[2] != "|") {
            continue;
        }

        //Update errorEntryTable Mapping
        std::string errorMessage;

        for(int index = 3; index < entries.size(); ++index) {
            errorMessage.append(entries[index] + " ");
        }

        this->errorEntryTable[entries[0]][entries[1]] = errorMessage;
    }
    textFile.close();
}


void ProgramParser::GenerateTranslationFromNode(const std::shared_ptr<SyntaxNode>& node) const
{
    // Recursively visits for children first, then itself (depth first search)
    for(auto& child : node->children) {
        GenerateTranslationFromNode(child);
    }

    if(this->compilerTranslationMap.find(node->rule) != this->compilerTranslationMap.end()) {
        auto translationRule = this->compilerTranslationMap.at(node->rule);
        for(const std::string& translationToken : translationRule) {
            // Check if it is a placeholder
            if(GrammarParser::IsNonTerminal(translationToken)) {
                // Get the child node that corresponds to the placeholder
                int foundIndex = -1;
                for(int index = 0; index < node->children.size(); index++) {
                    if(node->children.at(index)->rule.first == translationToken) {
                        foundIndex = index;
                        break;
                    }
                }

                if(foundIndex != -1) {
                    node->translation.append(node->children.at(foundIndex)->translation);
                }
            }
            else {
                // Otherwise just append
                std::string processedToken = ControlSequenceProcessor(translationToken);
                node->translation.append(processedToken);
            }
        }
    }
    else if(node->children.empty()) {
        // If this is a leaf node and does not have a translation mapping
        for(const std::string& ruleToken : node->rule.second) {
            if(ruleToken == RESERVE_WORD_LAMBDA) {
                // Ignore lambda
                continue;
            }
            node->translation.append(ruleToken);
        }
    }
    else {
        // If it is not a leaf node and does not have a translation mapping
        for(int index = 0; index < node->children.size(); index++) {
            node->translation.append(node->children.at(index)->translation);
        }
    }
}

std::string ProgramParser::ControlSequenceProcessor(const std::string& token)
{
    // Checks if it is a control character
    if(token.length() == 2 && token.front() == '\\') {
        switch(token.at(1)) {
            case 'n':
                return "\n";
            case 't':
                return "\t";
            case 's':
                return " ";
        }
    }
    else if(token.length() > 2 && token.front() == '\\' && token.at(1) == '<' && token.back() == '>') {
        // If a token is in form \<abcd>, remove the \.
        return token.substr(1) + " ";
    }
    // Otherwise just return the original string
    return token + " ";
}