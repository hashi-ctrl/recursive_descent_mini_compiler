#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include "lexer.h"

Token getNextToken(const char *input, int *position)
{
    Token token;
    token.lexeme[0] = '\0';

    /* Skip whitespace */
    while (isspace((unsigned char)input[*position]))
    {
        (*position)++;
    }

    /* End of input */
    if (input[*position] == '\0')
    {
        token.type = TOKEN_EOF;
        strcpy(token.lexeme, "EOF");
        return token;
    }

    char current = input[*position];

    /* Identifier or keyword */
    if (isalpha((unsigned char)current) || current == '_')
    {
        int start = *position;

        while (isalnum((unsigned char)input[*position]) ||
               input[*position] == '_')
        {
            (*position)++;
        }

        int length = *position - start;

        strncpy(token.lexeme, input + start, length);
        token.lexeme[length] = '\0';

        /* Check for keywords */
        if (strcmp(token.lexeme, "int") == 0)
        {
            token.type = TOKEN_INT;
        }
        else if (strcmp(token.lexeme, "float") == 0)
        {
            token.type = TOKEN_FLOAT;
        }
        else
        {
            token.type = TOKEN_IDENTIFIER;
        }

        return token;
    }

    /* Number */
    if (isdigit((unsigned char)current))
    {
        int start = *position;
        int hasDecimal = 0;

        while (isdigit((unsigned char)input[*position]))
        {
            (*position)++;
        }

        /* Handle decimal numbers */
        if (input[*position] == '.')
        {
            hasDecimal = 1;
            (*position)++;

            while (isdigit((unsigned char)input[*position]))
            {
                (*position)++;
            }
        }

        int length = *position - start;

        strncpy(token.lexeme, input + start, length);
        token.lexeme[length] = '\0';

        token.type = TOKEN_NUMBER;

        (void)hasDecimal; /* Reserved for future type handling */

        return token;
    }

    /* Operators and punctuation */
    (*position)++;

    switch (current)
    {
        case '+':
            token.type = TOKEN_PLUS;
            strcpy(token.lexeme, "+");
            break;

        case '-':
            token.type = TOKEN_MINUS;
            strcpy(token.lexeme, "-");
            break;

        case '*':
            token.type = TOKEN_MULTIPLY;
            strcpy(token.lexeme, "*");
            break;

        case '/':
            token.type = TOKEN_DIVIDE;
            strcpy(token.lexeme, "/");
            break;

        case '=':
            token.type = TOKEN_ASSIGN;
            strcpy(token.lexeme, "=");
            break;

        case '(':
            token.type = TOKEN_LPAREN;
            strcpy(token.lexeme, "(");
            break;

        case ')':
            token.type = TOKEN_RPAREN;
            strcpy(token.lexeme, ")");
            break;

        case ';':
            token.type = TOKEN_SEMICOLON;
            strcpy(token.lexeme, ";");
            break;

        default:
            token.type = TOKEN_UNKNOWN;
            token.lexeme[0] = current;
            token.lexeme[1] = '\0';
            break;
    }

    return token;
}
