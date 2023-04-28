#include "programparser.h"

int main()
{
    // Parser constructor creates a predictive parsing table from the grammar
    ProgramParser parser("grammar.txt", "error-entry-messages.txt");
    parser.CompileProgram("code.txt");
    return 0;
}
