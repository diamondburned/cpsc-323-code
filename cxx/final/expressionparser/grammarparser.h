
#ifndef PROJECTPREP_GRAMMARPARSER_H
#define PROJECTPREP_GRAMMARPARSER_H

#include <string>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <sstream>
#include <fstream>
#include <map>

class GrammarParser
{
public:
    typedef std::pair<std::string, std::vector<std::string>> GrammarEntry;
    typedef std::unordered_map<std::string, std::map<std::string, std::vector<std::string>>> ParsingTable;

    /**
     * Instantiates a new GrammarParser object.
     * Grammar from the text file must contain NO left-recursion, no ambiguity,
     * and perform left-factoring when possible.
     * @param fileLoc Text File Location of the grammar
     */
    explicit GrammarParser(const std::string& fileLoc);

    /**
     * Constructs a Predictive Parsing Table based on the grammar.
     * @return Parsing Table Structure.
     */
    ParsingTable ConstructPredictiveParsingTable() const;

    /**
     * Returns the Starting Grammar Rule from the grammar
     * @return Starting Grammar Rule.
     */
    const GrammarEntry& GetStartingGrammar() const {
        return this->grammar.front();
    }

    /**
     * Returns all of the terminals in the grammar.
     * @return Set of terminals.
     */
    const std::set<std::string>& GetTerminals() const {
        return this->terminalsSet;
    }

    /**
     * Returns all of the non-terminals in the grammar.
     * @return Set of non-terminals.
     */
    const std::unordered_set<std::string>& GetNonTerminals() const {
        return this->nonTerminalsSet;
    }

    /**
     * Checks if the token is a non-terminal. Non-terminals are defined using the '<' and '>' symbols.
     * @param token Token that will be checked
     * @return If the token is a non-terminal
     */
    bool static IsNonTerminal(const std::string& token) {
        return !token.empty() && token.at(0) == '<' && token.at(token.length() - 1) == '>';
    }

    /**
     * Prints the grammar from the file.
     */
    void PrintGrammar() const;

    /**
     * Prints the members of first of the grammar.
     */
    void PrintMembersOfFirst() const;

    /**
     * Prints the members of follow of the grammar.
     */
    void PrintMembersOfFollow() const;

    /**
     * Prints the predictive parsing table.
     */
    void PrintPredictiveParsingTable() const;

protected:

    /**
     * Method that computes the members of first of the given grammar.
     */
    void FindMembersOfFirst();

    /**
     * Helper function that computes the members of first of a given non-terminal.
     * @param nonTerminal Specified non-terminal
     */
    void FindFirstHelper(const std::string& nonTerminal);

    /**
     * Method that returns the members of first of a string of terminals/non-terminals.
     * Note: This method only works AFTER the members of first are computed for the grammar.
     * @param vector String of terminals and/or non-terminals
     * @return Set of first members
     */
    std::unordered_set<std::string> GetFirstFromRightSide(const std::vector<std::string>& vector) const;

    /**
     * Method that computes the members of follow of the given grammar.
     */
    void FindMembersOfFollow();

    /**
     * Helper function that computes the members of follow of a given non-terminal.
     * @param nonTerminal Specified non-terminal
     */
    void FindFollowHelper(const std::string& nonTerminal);

    /**
     * Replaces all instances of the non-terminal specified with their respective follow members.
     * @param placeholder Specified non-terminal
     */
    void ReplacePlaceholderInFollow(const std::string& placeholder);

    /**
     * Checks if there is a non-terminal within its follow set.
     * @param nonTerminal Specified non-terminal
     * @return If the follow set contains a non-terminal.
     */
    bool ContainsPlaceholderFollow(const std::string& nonTerminal) const;

private:
    std::string RESERVE_WORD_LAMBDA = "lambda";

    // Contains the list of grammar entries. Pair first = Left side of grammar, Pair second = Right side of grammar
    std::vector<GrammarEntry> grammar;

    // Defined by order in grammar read from file.
    std::vector<std::string> nonTerminalSetOrder;

    // Sets containing the non-terminals and terminals of the grammar.
    std::unordered_set<std::string> nonTerminalsSet;
    std::set<std::string> terminalsSet;

    // First and Follow Members of each non-terminal.
    std::unordered_map<std::string, std::set<std::string>> firstMembers;
    std::unordered_map<std::string, std::set<std::string>> followMembers;


    /**
     * Parses the grammar from the text file and initializes grammar elements within the class.
     * @param fileLoc Text File Location of the grammar.
     */
    void PrepareGrammarFromFile(const std::string& fileLoc);

};


GrammarParser::GrammarParser(const std::string& fileLoc)
{
    PrepareGrammarFromFile(fileLoc);
    FindMembersOfFirst();
    FindMembersOfFollow();
}

void GrammarParser::PrintGrammar() const
{
    for(auto& pair : this->grammar) {
        std::cout << pair.first << " -> ";

        for(const auto& token : pair.second) {
            std::cout << token << " ";
        }

        std::cout << std::endl;
    }
}

void GrammarParser::PrintMembersOfFirst() const
{
    std::cout << "====== Members of First ======" << std::endl;

    // Prints first members in order of non-terminal declarations in the original grammar file.
    for(const auto& nonTerminal : this->nonTerminalSetOrder) {
        if(this->firstMembers.find(nonTerminal) == this->firstMembers.end()) {
            std::cout << nonTerminal << ": {}" << std::endl;
            continue;
        }

        std::cout << nonTerminal << ": { ";
        for(const auto& terminal : this->firstMembers.at(nonTerminal)) {
            std::cout << terminal << ' ';
        }
        std::cout << "}" << std::endl;
    }
}

void GrammarParser::PrintMembersOfFollow() const
{
    std::cout << "====== Members of Follow ======" << std::endl;

    // Prints follow members in order of non-terminal declarations in the original grammar file.
    for(const auto& nonTerminal : this->nonTerminalSetOrder) {
        if(this->followMembers.find(nonTerminal) == this->followMembers.end()) {
            std::cout << nonTerminal << ": {}" << std::endl;
            continue;
        }

        std::cout << nonTerminal << ": { ";
        for(const auto& terminal : this->followMembers.at(nonTerminal)) {
            std::cout << terminal << ' ';
        }
        std::cout << "}" << std::endl;
    }
}

void GrammarParser::PrintPredictiveParsingTable() const
{
    auto table = ConstructPredictiveParsingTable();
    std::cout << "====== Predictive Parsing Table ======" << std::endl;

    // Prints rows in order of non-terminal declarations in the original grammar file.
    for(const auto& order : this->nonTerminalSetOrder) {
        const auto& nonTerminals = table.at(order);
        std::cout << "====== " << order << " ======\n";

        for(const auto& terminals : nonTerminals) {
            std::cout << "[ " << order << " , " << terminals.first << " ]:    ";

            std::string entry;
            for(const auto& token : terminals.second) {
                entry = entry + token + " ";
            }
            std::cout << entry.substr(0, entry.size() - 1) << std::endl;
        }
    }
}

GrammarParser::ParsingTable GrammarParser::ConstructPredictiveParsingTable() const
{
    ParsingTable table;

    //Constructs the predictive parsing table
    for(const auto& nonTerminal : this->nonTerminalSetOrder) {
        for(const auto& pair : this->grammar) {
            if(pair.first != nonTerminal) {
                continue;
            }

            if(pair.second.size() == 1 && pair.second.at(0) == RESERVE_WORD_LAMBDA) {
                // All y in Follow(A)
                for(const auto& terminal : this->followMembers.at(pair.first)) {
                    table[pair.first][terminal] = {RESERVE_WORD_LAMBDA};
                }
            }
            else if(pair.second.size() == 1 && !IsNonTerminal(pair.second.at(0))) {
                // alpha is terminal
                table[pair.first][pair.second.at(0)] = {pair.second.at(0)};
            }
            else {
                // if x is in First(B)
                for(const auto& terminal : GetFirstFromRightSide(pair.second)) {
                    if(terminal == RESERVE_WORD_LAMBDA) {
                        continue;
                    }
                    table[pair.first][terminal] = pair.second;
                }
            }
        }
    }
    return table;
}

void GrammarParser::FindMembersOfFirst()
{
    for(const auto& nonTerminal : this->nonTerminalsSet) {
        FindFirstHelper(nonTerminal);
    }
}

void GrammarParser::FindFirstHelper(const std::string& nonTerminal)
{
    if(!IsNonTerminal(nonTerminal)) {
        return;
    }

    // Searches through all rules of the grammar
    for(const auto& pair : this->grammar) {
        if(pair.first != nonTerminal) {
            continue;
        }

        if(!IsNonTerminal(pair.second.at(0))) {
            this->firstMembers[nonTerminal].insert(pair.second.at(0));
            continue;
        }

        bool lastLambda = false;
        for(const auto& token : pair.second) {
            if(IsNonTerminal(token)) {
                FindFirstHelper(token);

                // Adds all first members from the non-terminal.
                for(const auto& terminal : this->firstMembers.at(token)) {
                    if(terminal == RESERVE_WORD_LAMBDA) {
                        continue;
                    }
                    this->firstMembers[nonTerminal].insert(terminal);
                }

                //If the non-terminal contains no lambda, no need to recursive further
                if(this->firstMembers.at(token).find(RESERVE_WORD_LAMBDA) == this->firstMembers.at(token).end()) {
                    lastLambda = false;
                    break;
                }
                else
                {
                    lastLambda = true;
                }
            }
            else {
                // If encounter a terminal, add it to the member of first.
                this->firstMembers[nonTerminal].insert(token);
                lastLambda = false;
                break;
            }
        }

        //If the last non-terminal on the right side contains a lambda, that means lambda is a member of first.
        if(lastLambda) {
            this->firstMembers[nonTerminal].insert(RESERVE_WORD_LAMBDA);
        }
    }
}

std::unordered_set<std::string> GrammarParser::GetFirstFromRightSide(const std::vector<std::string>& vector) const
{
    std::unordered_set<std::string> firstSet;
    for(const auto& token : vector) {
        if(IsNonTerminal(token)) {

            //Adds all terminals from first(token)
            for(const auto& terminals : this->firstMembers.at(token)) {
                firstSet.insert(terminals);
            }

            //If lambda is not here, then don't continue to next terminal / non-terminal
            if(this->firstMembers.at(token).find(RESERVE_WORD_LAMBDA) == this->firstMembers.at(token).end()) {
                break;
            }
        }
        else {
            // Only insert first terminal to first set
            firstSet.insert(token);
            break;
        }
    }

    return firstSet;
}

void GrammarParser::FindMembersOfFollow()
{
    // First Rule
    // Starting grammar rule always has '$' as a member of follow.
    this->followMembers[this->nonTerminalSetOrder.front()].insert("$");

    for(const auto& nonTerminal : this->nonTerminalSetOrder) {
        FindFollowHelper(nonTerminal);
    }

    // Third Rule Post Processing, which replaces all non-terminals within the set
    bool done = false;
    while(!done) {
        done = true;
        for(const auto& pair : this->followMembers) {
            if(ContainsPlaceholderFollow(pair.first)) {
                done = false;
            }
            else {
                ReplacePlaceholderInFollow(pair.first);
            }
        }
    }
}

void GrammarParser::FindFollowHelper(const std::string& nonTerminal)
{
    if(!IsNonTerminal(nonTerminal)) {
        return;
    }

    // Second Rule
    // Adds the members of first of the non-terminal that directly follows the specified non-terminal.
    for(const auto& pair : this->grammar) {
        for(unsigned int index = 0; index < pair.second.size() - 1; ++index) {
            if(index + 1 >= pair.second.size()) {
                break;
            }

            const std::string& nextEntry = pair.second.at(index + 1);

            if(pair.second.at(index) != nonTerminal) {
                continue;
            }

            // Get First(B) where B are the tokens to the right.
            auto rightOfCurrent = std::vector<std::string>(pair.second.begin() + index + 1, pair.second.end());
            for(const auto& terminals : GetFirstFromRightSide(rightOfCurrent)) {
                if(terminals == RESERVE_WORD_LAMBDA) {
                    continue;
                }
                this->followMembers[nonTerminal].insert(terminals);
            }
        }
    }


    // Third Rule Pre Processing
    // This places non-terminals within the follow set if it needs to fetch the members of follow for that
    // non-terminal.
    for(const auto& pair : this->grammar) {
        bool previousTokenContainsLambda = true;
        for (auto iterator = pair.second.rbegin(); iterator != pair.second.rend(); ++iterator) {
            if (*iterator == nonTerminal && previousTokenContainsLambda) {
                //Mark this non-terminal to insert the follow(pair.first) into the set.
                if(pair.first != nonTerminal) {
                    this->followMembers[nonTerminal].insert(pair.first);
                }
                break;
            } else if (IsNonTerminal(*iterator)) {
                if (this->firstMembers[*iterator].find(RESERVE_WORD_LAMBDA) !=
                    this->firstMembers[*iterator].end()) {
                    previousTokenContainsLambda = true;
                } else {
                    previousTokenContainsLambda = false;
                }
            } else {
                // Only a terminal token is at the end of the grammar.
                break;
            }
        }
    }
}

void GrammarParser::ReplacePlaceholderInFollow(const std::string& placeholder)
{
    for(const auto& pair : this->followMembers) {
        if(pair.second.find(placeholder) == pair.second.end()) {
            continue;
        }

        this->followMembers[pair.first].erase(placeholder);
        for(const auto& terminals : this->followMembers[placeholder]) {
            this->followMembers[pair.first].insert(terminals);
        }
    }
}

bool GrammarParser::ContainsPlaceholderFollow(const std::string &nonTerminal) const
{
    for(const auto& terminal : this->followMembers.at(nonTerminal)) {
        if(IsNonTerminal(terminal)) {
            return true;
        }
    }
    return false;
}

void GrammarParser::PrepareGrammarFromFile(const std::string& fileLoc)
{
    std::ifstream textFile(fileLoc);
    if(!textFile || !textFile.is_open()) {
        std::cout << "Failed to open " << fileLoc << std::endl;
        throw std::invalid_argument("Failed to open file.");
    }
    this->grammar = std::vector<std::pair<std::string,std::vector<std::string>>>();

    std::string line;
    while(std::getline(textFile, line)) {
        std::vector<std::string> entries;
        std::stringstream stream(line);

        std::string value;
        while (std::getline(stream, value, ' ')) {
            entries.push_back(value);
        }
        if(entries.size() <= 2) {
            continue;
        }

        //Update non-terminal & terminal sets
        auto result = this->nonTerminalsSet.insert(entries[0]);
        if(result.second) {
            this->nonTerminalSetOrder.push_back(entries[0]);
        }
        for(auto iterator = entries.begin() + 2; iterator != entries.end(); ++iterator) {
            if(!IsNonTerminal(*iterator)) {
                this->terminalsSet.insert(*iterator);
            }
        }

        //Update Grammar Table
        if(entries.size() >= 3 && entries[1] == "->") {
            auto newEntries = std::vector<std::string>(entries.begin() + 2, entries.end());
            auto pair = std::pair<std::string, std::vector<std::string>>(entries[0], newEntries);

            this->grammar.push_back(pair);
            continue;
        }

        textFile.close();
        throw std::invalid_argument(line);
    }
    textFile.close();
}

#endif //PROJECTPREP_GRAMMARPARSER_H
