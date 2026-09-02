#include<stdio.h>

#include "lexer.h"

const char *tokenTypeToString(TokenType type)
{
    switch (type)
    {
        case TOKEN_INT:        return "INT";
        case TOKEN_FLOAT:      return "FLOAT";
        case TOKEN_IDENTIFIER: return "IDENTIFIER";
        case TOKEN_NUMBER:     return "NUMBER";
        case TOKEN_PLUS:       return "PLUS";
        case TOKEN_MINUS:      return "MINUS";
        case TOKEN_MULTIPLY:   return "MULTIPLY";
        case TOKEN_DIVIDE:     return "DIVIDE";
        case TOKEN_ASSIGN:     return "ASSIGN";
        case TOKEN_LPAREN:     return "LPAREN";
        case TOKEN_RPAREN:     return "RPAREN";
        case TOKEN_SEMICOLON:  return "SEMICOLON";
        case TOKEN_EOF:        return "EOF";
        case TOKEN_UNKNOWN:    return "UNKNOWN";
        default:               return "INVALID";
    }
}

int main(void)
{
    const char *input = "int x = 10 + 5; float y = x * 2.5;";

    int position = 0;
    Token token;

    printf("Input: %s\n\n", input);
    printf("%-15s %-15s\n", "TOKEN", "LEXEME");
    printf("--------------------------------\n");

    do
    {
        token = getNextToken(input, &position);

        printf("%-15s %-15s\n",
               tokenTypeToString(token.type),
               token.lexeme);

    } while (token.type != TOKEN_EOF);

    return 0;
}
