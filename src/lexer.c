

#include "lexer.h"
#include "dfa_lexer.h"

/*
    The lexical analyzer now uses the
    minimized DFA generated from the
    token regular expressions.

    The parser still calls getNextToken(),
    so the parser does not need to change.
*/

Token getNextToken(const char *input, int *position)
{
    return dfaGetNextToken(input, position);
}