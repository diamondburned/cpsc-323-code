#include "lib/programparser.hpp"

int main()
{
    // Parser constructor creates a predictive parsing table from the grammar
    ProgramParser parser("grammar.txt", "error-entry-messages.txt");

    parser.LoadCompilerTranslationFile("grammar-translation-cpp.txt");
    parser.CompileProgram("program.txt");
    return 0;
}
