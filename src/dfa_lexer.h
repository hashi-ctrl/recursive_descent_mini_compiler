#ifndef DFA_LEXER_H
#define DFA_LEXER_H

#include "token.h"

#define DFA_LEXEM_SIZE 100

/*
    Initialize the DFA-based lexer.
*/
void dfaLexerInit(void);

/*
    Get the next token from the source code.

    input    = complete source code
    position = current position in source
*/
Token dfaGetNextToken(const char *input, int *position);

#endif