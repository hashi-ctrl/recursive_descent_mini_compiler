#ifndef TOKEN_AUTOMATA_H
#define TOKEN_AUTOMATA_H

#include <stdio.h>

#define TA_MAX_NFA_STATES 512
#define TA_MAX_NFA_TRANSITIONS 2048
#define TA_MAX_DFA_STATES 512
#define TA_MAX_ALPHABET 128
#define TA_MAX_TOKEN_NAME 32

#define TA_EPSILON 128

typedef enum
{
    TA_TOKEN_NONE = 0,

    TA_TOKEN_INT,
    TA_TOKEN_FLOAT,
    TA_TOKEN_IDENTIFIER,
    TA_TOKEN_NUMBER,

    TA_TOKEN_PLUS,
    TA_TOKEN_MINUS,
    TA_TOKEN_MULTIPLY,
    TA_TOKEN_DIVIDE,
    TA_TOKEN_ASSIGN,

    TA_TOKEN_LPAREN,
    TA_TOKEN_RPAREN,
    TA_TOKEN_SEMICOLON,

    TA_TOKEN_UNKNOWN
} TATokenType;


typedef struct
{
    int from;
    int symbol;
    int to;
} TANFATransition;


typedef struct
{
    int state_count;
    int start_state;

    TANFATransition transitions[TA_MAX_NFA_TRANSITIONS];
    int transition_count;

    int accepting[TA_MAX_NFA_STATES];
    TATokenType token_type[TA_MAX_NFA_STATES];
    int priority[TA_MAX_NFA_STATES];
} TANFA;


typedef struct
{
    int state_count;
    int start_state;

    int transition[TA_MAX_DFA_STATES][TA_MAX_ALPHABET];

    int accepting[TA_MAX_DFA_STATES];
    TATokenType token_type[TA_MAX_DFA_STATES];
    int priority[TA_MAX_DFA_STATES];

    int subset[TA_MAX_DFA_STATES][TA_MAX_NFA_STATES];
} TADFA;


typedef struct
{
    int state_count;
    int start_state;

    int transition[TA_MAX_DFA_STATES][TA_MAX_ALPHABET];

    int accepting[TA_MAX_DFA_STATES];
    TATokenType token_type[TA_MAX_DFA_STATES];
    int priority[TA_MAX_DFA_STATES];
} TAMinDFA;


/* Build the complete token NFA */
void build_token_nfa(TANFA *nfa);

/* Convert NFA to DFA */
void token_nfa_to_dfa(const TANFA *nfa, TADFA *dfa);

/* Minimize DFA using partition refinement */
void minimize_token_dfa(const TADFA *dfa, TAMinDFA *min_dfa);

/* Print automata */
void print_token_nfa(const TANFA *nfa);
void print_token_dfa(const TADFA *dfa);
void print_min_token_dfa(const TAMinDFA *dfa);

/* Token names */
const char *token_type_name(TATokenType type);

/* Recognize one complete string */
TATokenType recognize_token(const TAMinDFA *dfa, const char *input);

#endif