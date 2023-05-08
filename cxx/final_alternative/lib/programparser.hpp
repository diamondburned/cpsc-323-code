
#ifndef PROJECTPREP_PROGRAMPARSER_H
#define PROJECTPREP_PROGRAMPARSER_H

#include <memory>
#include <stack>
#include <vector>
#include <string>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include "grammarparser.hpp"
#include "lexer.hpp"

class ProgramParser
{
public:
    typedef std::map<GrammarParser::GrammarEntry, std::vector<std::string>> TranslationMap;

    /**
     * Instantiates a new ProgramParser object.
     * @param fileLoc Text File Location of the grammar
     * @param errorInfoLoc Text File for the error state messages.
     */
    explicit ProgramParser(const std::string& grammarFileLoc, const std::string& errorEntriesLoc = "");

    /**
     * Loads the compiler translation file that contains the rules that translate the grammar into another syntax.
     * @param translationLoc Text File of the translation file.
     */
    void LoadCompilerTranslationFile(const std::string& translationLoc);

    /**
     * Compiles the given text file program.
     * @param inputFileLoc Text File of the program.
     */
    bool CompileProgram(const std::string& inputFileLoc) const;

protected:

    /**
     * SyntaxNode class that represents a node in the Syntax Tree.
     * This is formed when the compile method is parsing a given input. While the parse process is occurring,
     * a Syntax Tree is also being built, which is then used to generate the compiled code.
     */
    struct SyntaxNode : public std::enable_shared_from_this<SyntaxNode> {
        explicit SyntaxNode(std::shared_ptr<SyntaxNode> parentNode = nullptr) : parent{std::move(parentNode)} {}

        /**
         * Creates a new Child Node that belongs to the current parent node.
         * @return shared pointer of the new child node.
         */
        std::shared_ptr<SyntaxNode>& CreateChild(const GrammarParser::GrammarEntry& childRule) {
            auto child = std::make_shared<SyntaxNode>(shared_from_this());
            child->rule = childRule;
            this->children.insert(this->children.begin(), child);
            return this->children.front();
        }

        GrammarParser::GrammarEntry rule;
        std::string translation;
        std::shared_ptr<SyntaxNode> parent;
        std::vector<std::shared_ptr<SyntaxNode>> children;
    };

    /**
     * Loads the error entry message file into the parser. This specifies what type of
     * error messages are printed dependent on the invalid entry during the parse.
     * @param errorEntriesLoc Text File of the error entries.
     */
    void PrepareErrorEntriesFromFile(const std::string& errorEntriesLoc);

    /**
     * Compiles the nodes of the syntax tree into code syntax specified by the translation schema.
     * @param syntaxTreeRoot Root of the Syntax Tree.
     */
    std::string BuildCode(const std::shared_ptr<SyntaxNode>& syntaxTreeRoot) const {
        GenerateTranslationFromNode(syntaxTreeRoot);
        return syntaxTreeRoot->translation;
    }

    /**
     * A recursive helper function that performs a DFS and translates the Syntax Tree into the compiled code.
     * @param node Node of the Syntax Tree.
     */
    void GenerateTranslationFromNode(const std::shared_ptr<SyntaxNode>& node) const;

    /**
     * Converts a token that contains a control characters: {\n, \t, \s} into control character
     * form for building code. If the string is not a control character, it will just return the given token.
     * @param token String token that contains a control character.
     */
    static std::string ControlSequenceProcessor(const std::string& token);

private:
    std::string RESERVE_WORD_LAMBDA = "lambda";

    TranslationMap compilerTranslationMap;
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> errorEntryTable;

    std::unordered_map<std::string, std::map<std::string, std::vector<std::string>>> parsingTable;
    std::pair<std::string, std::vector<std::string>> startingGrammar;
    std::unordered_set<std::string> reserveWords;
    std::unordered_set<std::string> terminals;
};

#endif //PROJECTPREP_PROGRAMPARSER_H
