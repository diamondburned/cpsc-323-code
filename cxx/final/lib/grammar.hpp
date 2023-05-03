#pragma once

#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

const std::string LAMBDA = "λ";
const std::string SIGMA = "σ";

class Grammar {
 public:
  typedef std::pair<std::string, std::vector<std::string>> GrammarEntry;
  typedef std::unordered_map<std::string,
                             std::map<std::string, std::vector<std::string>>>
      ParsingTable;

  /**
   * Instantiates a new GrammarParser object.
   * Grammar from the text file must contain NO left-recursion, no ambiguity,
   * and perform left-factoring when possible.
   */
  Grammar(std::istream& grammarFile);
  Grammar(std::string grammarPath);

  /**
   * Constructs a Predictive Parsing Table based on the grammar.
   * @return Parsing Table Structure.
   */
  ParsingTable constructPredictiveParsingTable() const;

  /**
   * Returns the Starting Grammar Rule from the grammar
   * @return Starting Grammar Rule.
   */
  const GrammarEntry& getStartingGrammar() const {
    return this->grammar.front();
  }

  /**
   * Returns all of the terminals in the grammar.
   * @return Set of terminals.
   */
  const std::set<std::string>& getTerminals() const {
    return this->terminalsSet;
  }

  /**
   * Returns all of the non-terminals in the grammar.
   * @return Set of non-terminals.
   */
  const std::unordered_set<std::string>& getNonTerminals() const {
    return this->nonTerminalsSet;
  }

  /**
   * Checks if the token is a non-terminal. Non-terminals are defined using the
   * '<' and '>' symbols.
   * @param token Token that will be checked
   * @return If the token is a non-terminal
   */
  bool static isNonTerminal(const std::string& token) {
    return token.length() >= 2 && token.at(0) == '<' &&
           token.at(token.length() - 1) == '>';
  }

  bool static isTerminal(const std::string& token) {
    return !isNonTerminal(token);
  }

  /**
   * Prints the grammar from the file.
   */
  void printGrammar() const;

  /**
   * Prints the members of first of the grammar.
   */
  void printMembersOfFirst() const;

  /**
   * Prints the members of follow of the grammar.
   */
  void printMembersOfFollow() const;

  /**
   * Prints the predictive parsing table.
   */
  void printPredictiveParsingTable() const;

 protected:
  /**
   * Method that computes the members of first of the given grammar.
   */
  void findMembersOfFirst();

  /**
   * Helper function that computes the members of first of a given non-terminal.
   * @param nonTerminal Specified non-terminal
   */
  void findFirstHelper(const std::string& nonTerminal);

  /**
   * Method that returns the members of first of a string of
   * terminals/non-terminals. Note: This method only works AFTER the members of
   * first are computed for the grammar.
   * @param vector String of terminals and/or non-terminals
   * @return Set of first members
   */
  std::unordered_set<std::string> getFirstFromRightSide(
      const std::vector<std::string>& vector) const;

  /**
   * Method that computes the members of follow of the given grammar.
   */
  void findMembersOfFollow();

  /**
   * Helper function that computes the members of follow of a given
   * non-terminal.
   * @param nonTerminal Specified non-terminal
   */
  void findFollowHelper(const std::string& nonTerminal);

  /**
   * Replaces all instances of the non-terminal specified with their respective
   * follow members.
   * @param placeholder Specified non-terminal
   */
  void replacePlaceholderInFollow(const std::string& placeholder);

  /**
   * Checks if there is a non-terminal within its follow set.
   * @param nonTerminal Specified non-terminal
   * @return If the follow set contains a non-terminal.
   */
  bool containsPlaceholderFollow(const std::string& nonTerminal) const;

 private:
  // Contains the list of grammar entries. Pair first = Left side of grammar,
  // Pair second = Right side of grammar
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
   * Parses the grammar from the text file and initializes grammar elements
   * within the class.
   */
  void prepareGrammar(std::istream& file);
  void prepareGrammar(std::string path);

  // process processes the prepared grammar. Call this after prepareGrammar.
  void process();
};
