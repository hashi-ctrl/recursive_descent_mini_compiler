#include <stdio.h>

#include "dfa_lexer.h"


int main(void)
{
    const char *source =
        "int a; "
        "float value; "
        "a = value + 25 * 3;";


    int position = 0;

    Token token;


    printf("========================================\n");
    printf("       DFA BASED LEXICAL ANALYZER\n");
    printf("========================================\n\n");

    printf("SOURCE:\n");
    printf("%s\n\n", source);


    printf("TOKENS:\n");
    printf("----------------------------------------\n");


    dfaLexerInit();


    do
    {
        token =
            dfaGetNextToken(
                source,
                &position
            );


        printf(
            "%-15s -> %s\n",
            token.lexeme,
            token.type == TOKEN_INT ?
                "TOKEN_INT" :

            token.type == TOKEN_FLOAT ?
                "TOKEN_FLOAT" :

            token.type == TOKEN_IDENTIFIER ?
                "TOKEN_IDENTIFIER" :

            token.type == TOKEN_NUMBER ?
                "TOKEN_NUMBER" :

            token.type == TOKEN_PLUS ?
                "TOKEN_PLUS" :

            token.type == TOKEN_MINUS ?
                "TOKEN_MINUS" :

            token.type == TOKEN_MULTIPLY ?
                "TOKEN_MULTIPLY" :

            token.type == TOKEN_DIVIDE ?
                "TOKEN_DIVIDE" :

            token.type == TOKEN_ASSIGN ?
                "TOKEN_ASSIGN" :

            token.type == TOKEN_LPAREN ?
                "TOKEN_LPAREN" :

            token.type == TOKEN_RPAREN ?
                "TOKEN_RPAREN" :

            token.type == TOKEN_SEMICOLON ?
                "TOKEN_SEMICOLON" :

            token.type == TOKEN_EOF ?
                "TOKEN_EOF" :

                "TOKEN_UNKNOWN"
        );

    }
    while (token.type != TOKEN_EOF);


    return 0;
}