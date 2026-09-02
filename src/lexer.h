#ifndef LEXER_H
#define LEXER_H

#include "token.h"

Token getNextToken(const char *input, int *position);

#endif
