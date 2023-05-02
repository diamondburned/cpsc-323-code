
#ifndef PROJECTPREP_PROGRAMPARSER_H
#define PROJECTPREP_PROGRAMPARSER_H

#include <stack>

#include "grammarparser.h"

class ProgramParser {
 public:
  /**
   * Instantiates a new ProgramParser object.
   * @param fileLoc Text File Location of the grammar
   * @param errorInfoLoc Text File for the error state messages.
   */
  explicit ProgramParser(const std::string& grammarFileLoc,
                         const std::string& errorEntriesLoc = "");

  /**
   * Compiles the given text file program.
   * @param inputFileLoc Text File of the program.
   */
  bool CompileProgram(const std::string& inputFileLoc) const;

 protected:
  /**
   * Loads the error entry message file into the parser. This specifies what
   * type of error messages are printed dependent on the invalid entry during
   * the parse.
   * @param errorEntriesLoc Text File of the error entries.
   */
  void PrepareErrorEntriesFromFile(const std::string& errorEntriesLoc);

 private:
  std::string RESERVE_WORD_LAMBDA = "lambda";

  std::unordered_map<std::string, std::unordered_map<std::string, std::string>>
      errorEntryTable;

  std::unordered_map<std::string,
                     std::map<std::string, std::vector<std::string>>>
      parsingTable;
  std::pair<std::string, std::vector<std::string>> startingGrammar;

  std::unordered_set<std::string> reserveWords;
  std::unordered_set<std::string> terminals;
};

ProgramParser::ProgramParser(const std::string& grammarFileLoc,
                             const std::string& errorEntriesLoc) {
  GrammarParser grammar = GrammarParser(grammarFileLoc);
  this->parsingTable = grammar.ConstructPredictiveParsingTable();
  this->startingGrammar = grammar.GetStartingGrammar();

  // Keeps track of terminals and reserve words from the grammar
  for (const auto& terminal : grammar.GetTerminals()) {
    if (terminal.length() > 1) {
      this->reserveWords.insert(terminal);
    }
    this->terminals.insert(terminal);
  }

  if (!errorEntriesLoc.empty()) {
    PrepareErrorEntriesFromFile(errorEntriesLoc);
  }
}

bool ProgramParser::CompileProgram(const std::string& inputFileLoc) const {
  std::vector<std::string> tokens;
  std::ifstream textFile(inputFileLoc);
  if (!textFile || !textFile.is_open()) {
    std::cout << "Failed to open " << inputFileLoc << std::endl;
    return false;
  }

  // Creates vector of tokens from the prepared tokens file
  std::string line;
  while (std::getline(textFile, line)) {
    std::stringstream stream(line);
    std::string value;
    while (std::getline(stream, value, ' ')) {
      tokens.push_back(value);
    }
  }

  // Adds initials to the stack
  std::stack<std::string> parseStack;
  parseStack.push("$");
  parseStack.push(this->startingGrammar.first);

  int index = 0;
  while (!parseStack.empty() && index < tokens.size()) {
    std::string popped = parseStack.top();
    parseStack.pop();
    std::string readToken = tokens[index];

    // Splits Identifiers, Strings, etc. into multiple terminals
    if (readToken.length() > 1 &&
        this->reserveWords.find(readToken) == this->reserveWords.end()) {
      tokens.erase(tokens.begin() + index);
      for (auto iterator = readToken.rbegin(); iterator != readToken.rend();
           ++iterator) {
        tokens.insert(tokens.begin() + index, std::string(1, *iterator));
      }
      readToken = tokens.at(index);
    }

    if (this->terminals.find(popped) != this->terminals.end()) {
      if (readToken == popped) {
        // Matched, so advance to next character
        index++;
        continue;
      } else {
        // Program excepts a certain reserve word, got something else instead.
        std::cout << "Program contains Syntax Error." << std::endl;
        std::cout << popped << " was expected" << std::endl;
        return false;
      }
    }

    std::vector<std::string> tableEntry;
    try {
      tableEntry = this->parsingTable.at(popped).at(readToken);
    } catch (const std::exception& exception) {
      // Error, meaning we hit an invalid character or an empty entry in the
      // table. This means we read an incorrect entry from the parsing table.
      // Prints out Error Message
      std::cout << "Program contains Syntax Error." << std::endl;
      if (this->errorEntryTable.find(popped) != this->errorEntryTable.end()) {
        if (this->errorEntryTable.at(popped).find(readToken) !=
            this->errorEntryTable.at(popped).end()) {
          std::cout << this->errorEntryTable.at(popped).at(readToken)
                    << std::endl;
          return false;
        } else if (this->errorEntryTable.at(popped).find("?") !=
                   this->errorEntryTable.at(popped).end()) {
          // ? entry is an all encompassing entry, meaning all errors related to
          // the popped value is printed here.
          std::cout << this->errorEntryTable.at(popped).at("?") << std::endl;
          return false;
        }
      }
      std::cout << "Empty Entry at: "
                << "[" << popped << ", " << readToken << "]" << std::endl;
      std::cout << "Error at token " << index << std::endl;
      return false;
    }

    // Adds to stack based on the entry in the table.
    for (auto iterator = tableEntry.rbegin(); iterator != tableEntry.rend();
         ++iterator) {
      if (*iterator == RESERVE_WORD_LAMBDA) {
        // Ignore lambda terminals
        continue;
      }
      parseStack.push(*iterator);
    }
  }
  std::cout << "Program Accepted!" << std::endl;
  return true;
}

void ProgramParser::PrepareErrorEntriesFromFile(
    const std::string& errorEntriesLoc) {
  std::ifstream textFile(errorEntriesLoc);
  if (!textFile || !textFile.is_open()) {
    std::cout << "Failed to open " << errorEntriesLoc << std::endl;
    return;
  }

  std::string line;
  while (std::getline(textFile, line)) {
    std::vector<std::string> entries;
    std::stringstream stream(line);

    std::string value;
    while (std::getline(stream, value, ' ')) {
      entries.push_back(value);
    }
    if (entries.size() <= 3 || entries[2] != "|") {
      continue;
    }

    // Update errorEntryTable Mapping
    std::string errorMessage;

    for (int index = 3; index < entries.size(); ++index) {
      errorMessage.append(entries[index] + " ");
    }

    this->errorEntryTable[entries[0]][entries[1]] = errorMessage;
  }
  textFile.close();
}

#endif  // PROJECTPREP_PROGRAMPARSER_H
