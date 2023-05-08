
#ifndef PROJECTPREP_GRAMMARPARSER_H
#define PROJECTPREP_GRAMMARPARSER_H

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <set>
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

#endif //PROJECTPREP_GRAMMARPARSER_H
