#include <stdio.h>

#include "token_automata.h"


int main(void)
{
    TANFA nfa;
    TADFA dfa;
    TAMinDFA min_dfa;

    printf("============================================\n");
    printf("       TOKEN AUTOMATA LEXICAL ANALYZER\n");
    printf("============================================\n");

    printf("\nTOKEN REGULAR EXPRESSIONS\n");
    printf("--------------------------------------------\n");

    printf("INT        = int\n");
    printf("FLOAT      = float\n");
    printf("IDENTIFIER = [A-Za-z_][A-Za-z0-9_]*\n");
    printf("NUMBER     = [0-9]+(.[0-9]+)?\n");
    printf("PLUS       = +\n");
    printf("MINUS      = -\n");
    printf("MULTIPLY   = *\n");
    printf("DIVIDE     = /\n");
    printf("ASSIGN     = =\n");
    printf("LPAREN     = (\n");
    printf("RPAREN     = )\n");
    printf("SEMICOLON  = ;\n");


    /*
       STEP 1
       Build combined NFA
    */

    build_token_nfa(&nfa);

    printf("\n\n========== COMBINED NFA ==========\n");

    print_token_nfa(&nfa);


    /*
       STEP 2
       NFA -> DFA
    */

    token_nfa_to_dfa(&nfa, &dfa);

    printf("\n\n========== DFA ==========\n");

    print_token_dfa(&dfa);


    /*
       STEP 3
       DFA minimization
    */

    minimize_token_dfa(&dfa, &min_dfa);

    printf("\n\n========== MINIMIZED DFA ==========\n");

    print_min_token_dfa(&min_dfa);


    /*
       STEP 4
       Test individual lexemes
    */

    const char *tests[] =
    {
        "int",
        "float",
        "count",
        "_value",
        "student123",
        "123",
        "45.67",
        "+",
        "-",
        "*",
        "/",
        "=",
        "(",
        ")",
        ";",
        "abc123",
        "123abc",
        "hello!"
    };

    int test_count =
        sizeof(tests) / sizeof(tests[0]);


    printf("\n\n========== TOKEN RECOGNITION ==========\n");

    for (int i = 0;
         i < test_count;
         i++)
    {
        TATokenType type =
            recognize_token(
                &min_dfa,
                tests[i]
            );

        printf(
            "%-15s -> %s\n",
            tests[i],
            token_type_name(type)
        );
    }


    return 0;
}