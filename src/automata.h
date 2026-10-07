#ifndef AUTOMATA_H
#define AUTOMATA_H

#define MAX_NFA_STATES 256
#define MAX_DFA_STATES 256
#define MAX_REGEX_LEN 256

#define EPSILON 256

typedef struct {
    int from;
    int symbol;
    int to;
} NFATransition;

typedef struct {
    int state_count;
    int start_state;
    int accept_state;

    NFATransition transitions[1024];
    int transition_count;
} NFA;

typedef struct {
    int state_count;
    int start_state;

    int transition[MAX_DFA_STATES][128];

    int accepting[MAX_DFA_STATES];

    /* Each DFA state stores the NFA states represented by it */
    unsigned char subset[MAX_DFA_STATES][MAX_NFA_STATES];
} DFA;

typedef struct {
    int state_count;
    int start_state;

    int transition[MAX_DFA_STATES][128];
    int accepting[MAX_DFA_STATES];
} MinDFA;

/* Regex -> Thompson ε-NFA */
int regex_to_nfa(const char *regex, NFA *nfa);

/* ε-NFA -> DFA using subset construction */
int nfa_to_dfa(const NFA *nfa, DFA *dfa);

/* DFA minimization using partition refinement */
int minimize_dfa(const DFA *dfa, MinDFA *min);

/* Printing functions for demonstration/debugging */
void print_nfa(const NFA *nfa);
void print_dfa(const DFA *dfa);
void print_min_dfa(const MinDFA *dfa);

/* Test whether a string is accepted */
int dfa_accepts(const DFA *dfa, const char *input);
int min_dfa_accepts(const MinDFA *dfa, const char *input);

#endif