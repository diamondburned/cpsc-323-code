//------------------------------------------------------------------------------
// Group names: Ethan Safai, Erik Nguyen, Diamond Dinh
// Assignment : No. 5
// Due date   : 03/02/23
//
// Purpose: this program prepares a source code file for tokenization, removing
// comments, blank lines, and extra write space, and writes the result to a new
// file.
//------------------------------------------------------------------------------
#include <fstream>
#include <iostream>
#include <string>

// Removes comments (begin and end with "**" - without the quotes) from the 
// string
void removeComments(std::string &fileString);

// Trims the extra white space between tokens from a line, leaving the leading
// whitespace of the line
std::string trimLine(const std::string &line);

// Returns true if the line is empty, otherwise false
bool isEmptyLine(std::string &line);

// Removes extra whitespace, line by line, from the entire string
void removeExtraWhitespace(std::string &fileString);

// Prepares the source code file string for tokenization
void prepareForTokenization(std::string &fileString);

int main() {
  std::ifstream inFile("h5.txt");
  std::ofstream outFile;

  if (!inFile) {
    std::cerr << "File not found\n";
    exit(EXIT_FAILURE);
  }

  std::string fileString(
    (std::istreambuf_iterator<char>(inFile)),
    std::istreambuf_iterator<char>()
  );

  inFile.close();

  prepareForTokenization(fileString);
  
  std::cout << "******************** New file ********************\n";
  std::cout << fileString << std::endl;
  std::cout << "**************************************************\n";

  outFile.open("newh5.txt");
  outFile.write(fileString.c_str(), fileString.length());

  outFile.close();

  return 0;
}

void removeComments(std::string &fileString) {
  char currChar;
  size_t i = 0, n = fileString.length();
  std::string outString;

  while (i < n) {
    currChar = fileString[i];
    if (currChar == '*' && i + 1 < n && fileString[i + 1] == '*') {
      // find the end of the comment and skip over it
      i = fileString.find("**", i + 2);
      if (i == std::string::npos) {
        break;
      }
      i += 2;
      if (i >= n) {
        break;
      }
    } else {
      outString += currChar;
      ++i;
    }
  }
  fileString = outString;
}

std::string trimLine(const std::string &line) {
  std::string outString;

  size_t j = 0, m = line.length();
  // leave leading whitespace intact
  while (std::isspace(line[j])) {
    outString += line[j++];
  }

  // remove extra whitespace between tokens, while leaving at least one space if
  // any spaces were present
  while (j < m) {
    bool sawSpace = false;
    while (std::isspace(line[j])) {
      sawSpace = true;
      ++j;
    }
    if (sawSpace) {
      sawSpace = false;
      --j;
    }
    outString += line[j++];
  }
  outString += '\n';

  return outString;
}

bool isEmptyLine(std::string &line) {
  // check that each character is blank
  for (const char c : line) {
    if (!std::isspace(c)) {
      return false;
    }
  }
  return true;
}

void removeExtraWhitespace(std::string &fileString) {
  size_t i = 0, n;
  std::string currLine;
  std::string outString;

  while (true) {
    // find next line in the file string
    n = fileString.find('\n', i);
    // reached end of file
    if (n == std::string::npos)  {
      currLine = fileString.substr(i, fileString.length() - i);
      // trim line if not empty and add to out string
      if (!isEmptyLine(currLine)) {        
        outString += trimLine(currLine);
        // remove final newline character added by trimLine function
        outString.erase(outString.length() - 1);
      }
      break;
    }
    // trim line if not empty and add to out string
    currLine = fileString.substr(i, n - i);
    if (!isEmptyLine(currLine)) {
      outString += trimLine(currLine);
    }
  
    // slide window to next character after the newline character
    i = n + 1;
  }

  fileString = outString;
}

void prepareForTokenization(std::string &fileString) {
  removeComments(fileString);
  removeExtraWhitespace(fileString);
}