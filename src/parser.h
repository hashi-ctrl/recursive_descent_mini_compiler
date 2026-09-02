#ifndef PARSER_H
#define PARSER_H

#include "token.h"

typedef struct {
    const char *source;
    int position;
    Token current_token;
    int error_count;
    int temp_count;
} Parser;

/* Initialize the parser */
void parser_init(Parser *parser, const char *source);

/* Parse the complete program */
void parse_program(Parser *parser);

/* Check whether parsing completed successfully */
int parser_has_errors(const Parser *parser);

#endif
