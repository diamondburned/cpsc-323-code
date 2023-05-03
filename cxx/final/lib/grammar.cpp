#include "grammar.hpp"

#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

Grammar::Grammar(std::istream& grammarFile) {
  prepareGrammar(grammarFile);
  process();
}

Grammar::Grammar(std::string grammarPath) {
  std::ifstream grammarFile(grammarPath);
  prepareGrammar(grammarFile);
  process();
}

void Grammar::process() {
  findMembersOfFirst();
  findMembersOfFollow();
}

void Grammar::printGrammar() const {
  for (auto& pair : grammar) {
    std::cerr << pair.first << " -> ";

    for (const auto& token : pair.second) {
      std::cerr << token << " ";
    }

    std::cerr << std::endl;
  }
}

void Grammar::printMembersOfFirst() const {
  std::cerr << "====== Members of First ======" << std::endl;

  // Prints first members in order of non-terminal declarations in the original
  // grammar file.
  for (const auto& nonTerminal : nonTerminalSetOrder) {
    if (firstMembers.find(nonTerminal) == firstMembers.end()) {
      std::cerr << nonTerminal << ": {}" << std::endl;
      continue;
    }

    std::cerr << nonTerminal << ": { ";
    for (const auto& terminal : firstMembers.at(nonTerminal)) {
      std::cerr << terminal << ' ';
    }
    std::cerr << "}" << std::endl;
  }
}

void Grammar::printMembersOfFollow() const {
  std::cerr << "====== Members of Follow ======" << std::endl;

  // Prints follow members in order of non-terminal declarations in the original
  // grammar file.
  for (const auto& nonTerminal : nonTerminalSetOrder) {
    if (followMembers.find(nonTerminal) == followMembers.end()) {
      std::cerr << nonTerminal << ": {}" << std::endl;
      continue;
    }

    std::cerr << nonTerminal << ": { ";
    for (const auto& terminal : followMembers.at(nonTerminal)) {
      std::cerr << terminal << ' ';
    }
    std::cerr << "}" << std::endl;
  }
}

void Grammar::printPredictiveParsingTable() const {
  auto table = constructPredictiveParsingTable();
  std::cerr << "====== Predictive Parsing Table ======" << std::endl;

  // Prints rows in order of non-terminal declarations in the original grammar
  // file.
  for (const auto& order : nonTerminalSetOrder) {
    const auto& nonTerminals = table.at(order);
    std::cerr << "====== " << order << " ======\n";

    for (const auto& terminals : nonTerminals) {
      std::cerr << "[ " << order << " , " << terminals.first << " ]:    ";

      std::string entry;
      for (const auto& token : terminals.second) {
        entry = entry + token + " ";
      }
      std::cerr << entry.substr(0, entry.size() - 1) << std::endl;
    }
  }
}

Grammar::ParsingTable Grammar::constructPredictiveParsingTable() const {
  ParsingTable table;

  // Constructs the predictive parsing table
  for (const auto& nonTerminal : nonTerminalSetOrder) {
    for (const auto& pair : grammar) {
      if (pair.first != nonTerminal) {
        continue;
      }

      if (pair.second.size() == 1 && pair.second.at(0) == LAMBDA) {
        // All y in Follow(A)
        for (const auto& terminal : followMembers.at(pair.first)) {
          table[pair.first][terminal] = {LAMBDA};
        }
      } else if (pair.second.size() == 1 && !isNonTerminal(pair.second.at(0))) {
        // alpha is terminal
        table[pair.first][pair.second.at(0)] = {pair.second.at(0)};
      } else {
        // if x is in First(B)
        for (const auto& terminal : getFirstFromRightSide(pair.second)) {
          if (terminal == LAMBDA) {
            continue;
          }
          table[pair.first][terminal] = pair.second;
        }
      }
    }
  }
  return table;
}

void Grammar::findMembersOfFirst() {
  for (const auto& nonTerminal : nonTerminalsSet) {
    findFirstHelper(nonTerminal);
  }
}

void Grammar::findFirstHelper(const std::string& nonTerminal) {
  if (!isNonTerminal(nonTerminal)) {
    return;
  }

  // Searches through all rules of the grammar
  for (const auto& pair : grammar) {
    if (pair.first != nonTerminal) {
      continue;
    }

    if (!isNonTerminal(pair.second.at(0))) {
      firstMembers[nonTerminal].insert(pair.second.at(0));
      continue;
    }

    bool lastLambda = false;
    for (const auto& token : pair.second) {
      if (isNonTerminal(token)) {
        findFirstHelper(token);

        // Adds all first members from the non-terminal.
        for (const auto& terminal : firstMembers.at(token)) {
          if (terminal == LAMBDA) {
            continue;
          }
          firstMembers[nonTerminal].insert(terminal);
        }

        // If the non-terminal contains no LAMBDA, no need to recursive further
        if (firstMembers.at(token).find(LAMBDA) ==
            firstMembers.at(token).end()) {
          lastLambda = false;
          break;
        } else {
          lastLambda = true;
        }
      } else {
        // If encounter a terminal, add it to the member of first.
        firstMembers[nonTerminal].insert(token);
        lastLambda = false;
        break;
      }
    }

    // If the last non-terminal on the right side contains a LAMBDA, that means
    // LAMBDA is a member of first.
    if (lastLambda) {
      firstMembers[nonTerminal].insert(LAMBDA);
    }
  }
}

std::unordered_set<std::string> Grammar::getFirstFromRightSide(
    const std::vector<std::string>& vector) const {
  std::unordered_set<std::string> firstSet;
  for (const auto& token : vector) {
    if (isNonTerminal(token)) {
      // Adds all terminals from first(token)
      for (const auto& terminals : firstMembers.at(token)) {
        firstSet.insert(terminals);
      }

      // If LAMBDA is not here, then don't continue to next terminal /
      // non-terminal
      if (firstMembers.at(token).find(LAMBDA) == firstMembers.at(token).end()) {
        break;
      }
    } else {
      // Only insert first terminal to first set
      firstSet.insert(token);
      break;
    }
  }

  return firstSet;
}

void Grammar::findMembersOfFollow() {
  // First Rule
  // Starting grammar rule always has '$' as a member of follow.
  followMembers[nonTerminalSetOrder.front()].insert("$");

  for (const auto& nonTerminal : nonTerminalSetOrder) {
    findFollowHelper(nonTerminal);
  }

  // Third Rule Post Processing, which replaces all non-terminals within the set
  bool done = false;
  while (!done) {
    done = true;
    for (const auto& pair : followMembers) {
      if (containsPlaceholderFollow(pair.first)) {
        done = false;
      } else {
        replacePlaceholderInFollow(pair.first);
      }
    }
  }
}

void Grammar::findFollowHelper(const std::string& nonTerminal) {
  if (!isNonTerminal(nonTerminal)) {
    return;
  }

  // Second Rule
  // Adds the members of first of the non-terminal that directly follows the
  // specified non-terminal.
  for (const auto& pair : grammar) {
    for (unsigned int index = 0; index < pair.second.size() - 1; ++index) {
      if (index + 1 >= pair.second.size()) {
        break;
      }

      if (pair.second.at(index) != nonTerminal) {
        continue;
      }

      // Get First(B) where B are the tokens to the right.
      auto rightOfCurrent = std::vector<std::string>(
          pair.second.begin() + index + 1, pair.second.end());
      for (const auto& terminals : getFirstFromRightSide(rightOfCurrent)) {
        if (terminals == LAMBDA) {
          continue;
        }
        followMembers[nonTerminal].insert(terminals);
      }
    }
  }

  // Third Rule Pre Processing
  // This places non-terminals within the follow set if it needs to fetch the
  // members of follow for that non-terminal.
  for (const auto& pair : grammar) {
    bool previousTokenContainsLambda = true;
    for (auto iterator = pair.second.rbegin(); iterator != pair.second.rend();
         ++iterator) {
      if (*iterator == nonTerminal && previousTokenContainsLambda) {
        // Mark this non-terminal to insert the follow(pair.first) into the set.
        if (pair.first != nonTerminal) {
          followMembers[nonTerminal].insert(pair.first);
        }
        break;
      } else if (isNonTerminal(*iterator)) {
        if (firstMembers[*iterator].find(LAMBDA) !=
            firstMembers[*iterator].end()) {
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

void Grammar::replacePlaceholderInFollow(const std::string& placeholder) {
  for (const auto& pair : followMembers) {
    if (pair.second.find(placeholder) == pair.second.end()) {
      continue;
    }

    followMembers[pair.first].erase(placeholder);
    for (const auto& terminals : followMembers[placeholder]) {
      followMembers[pair.first].insert(terminals);
    }
  }
}

bool Grammar::containsPlaceholderFollow(const std::string& nonTerminal) const {
  for (const auto& terminal : followMembers.at(nonTerminal)) {
    if (isNonTerminal(terminal)) {
      return true;
    }
  }
  return false;
}

void Grammar::prepareGrammar(std::string path) {
  std::ifstream f(path);
  prepareGrammar(f);
}

void Grammar::prepareGrammar(std::istream& file) {
  grammar = std::vector<std::pair<std::string, std::vector<std::string>>>();

  std::string line;
  while (std::getline(file, line)) {
    std::vector<std::string> entries;
    std::stringstream stream(line);

    std::string value;
    while (std::getline(stream, value, ' ')) {
      entries.push_back(value);
    }
    if (entries.size() <= 2) {
      continue;
    }

    // Update non-terminal & terminal sets
    auto result = nonTerminalsSet.insert(entries[0]);
    if (result.second) {
      nonTerminalSetOrder.push_back(entries[0]);
    }
    for (auto iterator = entries.begin() + 2; iterator != entries.end();
         ++iterator) {
      if (isTerminal(*iterator)) {
        terminalsSet.insert(*iterator);
      }
    }

    // Update Grammar Table
    if (entries.size() >= 3 && entries[1] == "->") {
      auto newEntries =
          std::vector<std::string>(entries.begin() + 2, entries.end());
      auto pair = std::pair<std::string, std::vector<std::string>>(entries[0],
                                                                   newEntries);

      grammar.push_back(pair);
      continue;
    }

    throw std::invalid_argument(line);
  }
}
