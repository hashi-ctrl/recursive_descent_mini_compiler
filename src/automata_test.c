#include <stdio.h>
#include "automata.h"

int main(void)
{
    const char *regex = "(a|b)*abb";

    NFA nfa;
    DFA dfa;
    MinDFA min_dfa;

    printf("========================================\n");
    printf("       REGEX → NFA → DFA → MIN DFA\n");
    printf("========================================\n\n");

    printf("Regular Expression: %s\n\n", regex);

    /* Step 1: Regex -> NFA */
    regex_to_nfa(regex, &nfa);

    printf("========== NFA ==========\n");
    print_nfa(&nfa);

    /* Step 2: NFA -> DFA */
    nfa_to_dfa(&nfa, &dfa);

    printf("\n========== DFA ==========\n");
    print_dfa(&dfa);

    /* Step 3: DFA minimization */
    minimize_dfa(&dfa, &min_dfa);

    printf("\n====== MINIMIZED DFA ======\n");
    print_min_dfa(&min_dfa);

    /* Test strings */
    const char *tests[] = {
        "abb",
        "aabb",
        "ababb",
        "abababb",
        "ab",
        "abc",
        "aab"
    };

    int test_count = sizeof(tests) / sizeof(tests[0]);

    printf("\n========== TEST RESULTS ==========\n");

    for (int i = 0; i < test_count; i++)
    {
        int result = min_dfa_accepts(&min_dfa, tests[i]);

        printf("%-10s : %s\n",
               tests[i],
               result ? "ACCEPT" : "REJECT");
    }

    return 0;
}