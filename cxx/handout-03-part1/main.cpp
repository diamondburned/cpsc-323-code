//------------------------------------------------------------------------------
// Group names: Ethan Safai, Erik Nguyen, Diamond Dinh
// Assignment : No. 3
// Due date   : 02/16/23
//
// Purpose: this program reads a file of tokens and outputs whether each token
// is a number, identifier, reserved word, or none of these.
//------------------------------------------------------------------------------
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>

enum TokenType { RESERVED, IDENTIFIER, NUMBER, INVALID };

const int NUM_RESERVED = 5;
const std::string reservedWords[NUM_RESERVED] = { 
  "while", "for", "switch", "do", "return"
};

// Returns true if s is a reserved word, otherwise false
bool isReserved(const std::string& s);

// Returns true if s is numeric, otherwise false
bool isNumber(const std::string& s);

// Returns true if s is a valid identifier, otherwise false
bool isValidIdentifier(const std::string& s);

// Returns the token type of tok
// Pre-conditions: tok is at least of length 1
TokenType getTokenType(const std::string& tok);

// Read each token from the file, line by line, and output the token type to the
// screen
int main() {
  std::ifstream inFile("tokens.txt");
  std::string currToken;

  if (inFile) {
    std::left(std::cout);
    std::cout << std::setw(12) << "Token" << std::setw(12) << "number";
    std::cout << std::setw(12) << "identifier" << "reserved word\n";
    std::cout << "-------------------------------------------------\n";

    // read each token from the file, line by line
    while (std::getline(inFile, currToken)) {
      std::cout << std::setw(12) << currToken;

      // compute and display the type of the current token
      switch (getTokenType(currToken)) {
      case RESERVED: 
        std::cout << std::setw(12) << "no" << std::setw(12) << "no" 
                  << "yes\n";
        break;
      case NUMBER:
        std::cout << std::setw(12) << "yes" << std::setw(12) << "no" 
                  << "no\n";
        break;
      case IDENTIFIER:
        std::cout << std::setw(12) << "no" << std::setw(12) << "yes" 
                  << "no\n";
        break;
      default:
        std::cout << std::setw(12) << "no" << std::setw(12) << "no" 
                  << "no\n";
        break;
      }
    }

    inFile.close();
  } else {
    std::cout << "File not found\n";
    exit(EXIT_FAILURE);
  }

  return 0;
}

bool isReserved(const std::string& s) {
  // compare s to each word in the reserved array
  for (int i = 0; i < NUM_RESERVED; i++) {
    if (s == reservedWords[i]) {
      return true;
    }
  }
  return false;
}

bool isNumber(const std::string& s) {
  // if each character is a digit, the token is a number
  for (const char c : s) {
    if (!std::isdigit(c)) {
      return false;
    }
  }
  return true;
}

bool isValidIdentifier(const std::string& s) {
  // first character must be an underscore or letter
  if (s[0] != '_' && !std::isalpha(s[0])) {
    return false;
  }

  for (size_t i = 1, n = s.length(); i < n; i++) {
    if (s[i] != '_' && !std::isalnum(s[i])) {
      return false;
    }
  }
  return true;
}

TokenType getTokenType(const std::string& tok) {
  if (isReserved(tok)) {
    return RESERVED;
  }

  if (isNumber(tok)) {
    return NUMBER;
  }

  if (isValidIdentifier(tok)) {
    return IDENTIFIER;
  }

  // if token is not reserved, numeric, or an identifier, then it is invalid
  return INVALID;
}