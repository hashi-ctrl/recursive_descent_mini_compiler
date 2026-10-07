#include "token_automata.h"

#include <string.h>
#include <ctype.h>


/* ============================================================
   BASIC NFA FUNCTIONS
   ============================================================ */

static int new_state(TANFA *nfa)
{
    if (nfa->state_count >= TA_MAX_NFA_STATES)
        return -1;

    return nfa->state_count++;
}


static void add_transition(TANFA *nfa, int from, int symbol, int to)
{
    if (nfa->transition_count >= TA_MAX_NFA_TRANSITIONS)
        return;

    nfa->transitions[nfa->transition_count].from = from;
    nfa->transitions[nfa->transition_count].symbol = symbol;
    nfa->transitions[nfa->transition_count].to = to;

    nfa->transition_count++;
}


static void add_char_transition(
    TANFA *nfa,
    int from,
    char ch,
    int to)
{
    add_transition(nfa, from, (unsigned char)ch, to);
}


static void add_epsilon(
    TANFA *nfa,
    int from,
    int to)
{
    add_transition(nfa, from, TA_EPSILON, to);
}


/* ============================================================
   TOKEN RULE BUILDERS
   ============================================================ */

/*
    We use the following lexical regular expressions:

    INT         = int
    FLOAT       = float

    IDENTIFIER  = [A-Za-z_][A-Za-z0-9_]*
    NUMBER      = [0-9]+(\.[0-9]+)?

    PLUS        = +
    MINUS       = -
    MULTIPLY    = *
    DIVIDE      = /
    ASSIGN      = =
    LPAREN      = (
    RPAREN      = )
    SEMICOLON   = ;

    These are represented as Thompson-style NFA fragments.
*/


static int create_accept_state(
    TANFA *nfa,
    TATokenType token,
    int priority)
{
    int state = new_state(nfa);

    if (state >= 0)
    {
        nfa->accepting[state] = 1;
        nfa->token_type[state] = token;
        nfa->priority[state] = priority;
    }

    return state;
}


/* Add literal string: e.g. "int", "float" */
static void add_literal_rule(
    TANFA *nfa,
    int start,
    const char *text,
    TATokenType token,
    int priority)
{
    int current = start;

    for (int i = 0; text[i] != '\0'; i++)
    {
        int next = new_state(nfa);

        add_char_transition(
            nfa,
            current,
            text[i],
            next
        );

        current = next;
    }

    nfa->accepting[current] = 1;
    nfa->token_type[current] = token;
    nfa->priority[current] = priority;
}


/*
    IDENTIFIER:

    [A-Za-z_][A-Za-z0-9_]*
*/
static void add_identifier_rule(
    TANFA *nfa,
    int start,
    TATokenType token,
    int priority)
{
    int loop = new_state(nfa);
    int accept = loop;

    /* First character: letter or underscore */

    for (char c = 'A'; c <= 'Z'; c++)
        add_char_transition(nfa, start, c, loop);

    for (char c = 'a'; c <= 'z'; c++)
        add_char_transition(nfa, start, c, loop);

    add_char_transition(nfa, start, '_', loop);

    /*
       Remaining characters:
       letters, digits, underscore
    */

    for (char c = 'A'; c <= 'Z'; c++)
        add_char_transition(nfa, loop, c, loop);

    for (char c = 'a'; c <= 'z'; c++)
        add_char_transition(nfa, loop, c, loop);

    for (char c = '0'; c <= '9'; c++)
        add_char_transition(nfa, loop, c, loop);

    add_char_transition(nfa, loop, '_', loop);

    nfa->accepting[accept] = 1;
    nfa->token_type[accept] = token;
    nfa->priority[accept] = priority;
}


/*
    NUMBER:

    [0-9]+(\.[0-9]+)?

    This recognizes:

        123
        45
        12.5
        3.14159
*/
static void add_number_rule(
    TANFA *nfa,
    int start,
    TATokenType token,
    int priority)
{
    int digits = new_state(nfa);
    int dot = new_state(nfa);
    int decimal = new_state(nfa);

    /* First digit */
    for (char c = '0'; c <= '9'; c++)
        add_char_transition(nfa, start, c, digits);

    /* More integer digits */
    for (char c = '0'; c <= '9'; c++)
        add_char_transition(nfa, digits, c, digits);

    /*
       Decimal point.
    */
    add_char_transition(nfa, digits, '.', dot);

    /*
       First digit after decimal point.
    */
    for (char c = '0'; c <= '9'; c++)
        add_char_transition(nfa, dot, c, decimal);

    /*
       More decimal digits.
    */
    for (char c = '0'; c <= '9'; c++)
        add_char_transition(nfa, decimal, c, decimal);

    /*
       Both integer and decimal states are accepting.

       123     -> NUMBER
       123.45  -> NUMBER
    */

    nfa->accepting[digits] = 1;
    nfa->token_type[digits] = token;
    nfa->priority[digits] = priority;

    nfa->accepting[decimal] = 1;
    nfa->token_type[decimal] = token;
    nfa->priority[decimal] = priority;
}


/* Single-character token */
static void add_single_char_rule(
    TANFA *nfa,
    int start,
    char ch,
    TATokenType token,
    int priority)
{
    int accept = new_state(nfa);

    add_char_transition(nfa, start, ch, accept);

    nfa->accepting[accept] = 1;
    nfa->token_type[accept] = token;
    nfa->priority[accept] = priority;
}


/* ============================================================
   BUILD COMPLETE TOKEN NFA
   ============================================================ */

void build_token_nfa(TANFA *nfa)
{
    memset(nfa, 0, sizeof(TANFA));

    nfa->start_state = new_state(nfa);

    /*
       Lower number = higher priority.

       Keywords get higher priority than IDENTIFIER.
    */

    add_literal_rule(
        nfa,
        nfa->start_state,
        "int",
        TA_TOKEN_INT,
        1
    );

    add_literal_rule(
        nfa,
        nfa->start_state,
        "float",
        TA_TOKEN_FLOAT,
        1
    );

    add_identifier_rule(
        nfa,
        nfa->start_state,
        TA_TOKEN_IDENTIFIER,
        3
    );

    add_number_rule(
        nfa,
        nfa->start_state,
        TA_TOKEN_NUMBER,
        2
    );

    add_single_char_rule(
        nfa,
        nfa->start_state,
        '+',
        TA_TOKEN_PLUS,
        4
    );

    add_single_char_rule(
        nfa,
        nfa->start_state,
        '-',
        TA_TOKEN_MINUS,
        4
    );

    add_single_char_rule(
        nfa,
        nfa->start_state,
        '*',
        TA_TOKEN_MULTIPLY,
        4
    );

    add_single_char_rule(
        nfa,
        nfa->start_state,
        '/',
        TA_TOKEN_DIVIDE,
        4
    );

    add_single_char_rule(
        nfa,
        nfa->start_state,
        '=',
        TA_TOKEN_ASSIGN,
        4
    );

    add_single_char_rule(
        nfa,
        nfa->start_state,
        '(',
        TA_TOKEN_LPAREN,
        4
    );

    add_single_char_rule(
        nfa,
        nfa->start_state,
        ')',
        TA_TOKEN_RPAREN,
        4
    );

    add_single_char_rule(
        nfa,
        nfa->start_state,
        ';',
        TA_TOKEN_SEMICOLON,
        4
    );
}


/* ============================================================
   EPSILON CLOSURE
   ============================================================ */

static void epsilon_closure(
    const TANFA *nfa,
    const int *input,
    int input_count,
    int *output,
    int *output_count)
{
    int visited[TA_MAX_NFA_STATES] = {0};
    int stack[TA_MAX_NFA_STATES];
    int top = 0;

    *output_count = 0;

    for (int i = 0; i < input_count; i++)
    {
        int s = input[i];

        if (!visited[s])
        {
            visited[s] = 1;
            stack[top++] = s;
        }
    }

    while (top > 0)
    {
        int state = stack[--top];

        output[(*output_count)++] = state;

        for (int i = 0; i < nfa->transition_count; i++)
        {
            TANFATransition t = nfa->transitions[i];

            if (t.from == state &&
                t.symbol == TA_EPSILON &&
                !visited[t.to])
            {
                visited[t.to] = 1;
                stack[top++] = t.to;
            }
        }
    }
}


/* ============================================================
   MOVE
   ============================================================ */

static void move_states(
    const TANFA *nfa,
    const int *states,
    int state_count,
    int symbol,
    int *output,
    int *output_count)
{
    int visited[TA_MAX_NFA_STATES] = {0};

    *output_count = 0;

    for (int i = 0; i < state_count; i++)
    {
        int state = states[i];

        for (int j = 0; j < nfa->transition_count; j++)
        {
            TANFATransition t = nfa->transitions[j];

            if (t.from == state &&
                t.symbol == symbol &&
                !visited[t.to])
            {
                visited[t.to] = 1;
                output[(*output_count)++] = t.to;
            }
        }
    }
}


/* ============================================================
   DFA HELPERS
   ============================================================ */

static int same_subset(
    const int *a,
    int a_count,
    const int *b,
    int b_count)
{
    if (a_count != b_count)
        return 0;

    for (int i = 0; i < a_count; i++)
    {
        int found = 0;

        for (int j = 0; j < b_count; j++)
        {
            if (a[i] == b[j])
            {
                found = 1;
                break;
            }
        }

        if (!found)
            return 0;
    }

    return 1;
}


static int find_dfa_state(
    const TADFA *dfa,
    const int *subset,
    int count)
{
    for (int i = 0; i < dfa->state_count; i++)
    {
        if (same_subset(
                dfa->subset[i],
                TA_MAX_NFA_STATES,
                subset,
                count))
        {
            return i;
        }
    }

    return -1;
}


/* ============================================================
   DFA ACCEPTANCE LABEL
   ============================================================ */

static void determine_dfa_token(
    const TANFA *nfa,
    const int *subset,
    int *accepting,
    TATokenType *token,
    int *priority)
{
    *accepting = 0;
    *token = TA_TOKEN_NONE;
    *priority = 999999;

    for (int i = 0; i < TA_MAX_NFA_STATES; i++)
    {
        int state = subset[i];

        if (state < 0 || state >= nfa->state_count)
            continue;

        if (nfa->accepting[state])
        {
            int p = nfa->priority[state];

            if (!(*accepting) || p < *priority)
            {
                *accepting = 1;
                *token = nfa->token_type[state];
                *priority = p;
            }
        }
    }
}


/* ============================================================
   NFA -> DFA
   ============================================================ */

void token_nfa_to_dfa(
    const TANFA *nfa,
    TADFA *dfa)
{
    memset(dfa, 0, sizeof(TADFA));

    dfa->start_state = 0;

    int start_input[1] = { nfa->start_state };
    int start_closure[TA_MAX_NFA_STATES];
    int start_count = 0;

    epsilon_closure(
        nfa,
        start_input,
        1,
        start_closure,
        &start_count
    );

    /*
       Store subset.

       We fill unused entries with -1.
    */

    for (int i = 0; i < TA_MAX_NFA_STATES; i++)
        dfa->subset[0][i] = -1;

    for (int i = 0; i < start_count; i++)
        dfa->subset[0][i] = start_closure[i];

    dfa->state_count = 1;

    determine_dfa_token(
        nfa,
        dfa->subset[0],
        &dfa->accepting[0],
        &dfa->token_type[0],
        &dfa->priority[0]
    );

    int current = 0;

    while (current < dfa->state_count)
    {
        int current_states[TA_MAX_NFA_STATES];
        int current_count = 0;

        for (int i = 0; i < TA_MAX_NFA_STATES; i++)
        {
            if (dfa->subset[current][i] != -1)
            {
                current_states[current_count++] =
                    dfa->subset[current][i];
            }
        }

        for (int symbol = 0; symbol < TA_MAX_ALPHABET; symbol++)
        {
            int moved[TA_MAX_NFA_STATES];
            int moved_count = 0;

            int closure[TA_MAX_NFA_STATES];
            int closure_count = 0;

            move_states(
                nfa,
                current_states,
                current_count,
                symbol,
                moved,
                &moved_count
            );

            if (moved_count == 0)
            {
                dfa->transition[current][symbol] = -1;
                continue;
            }

            epsilon_closure(
                nfa,
                moved,
                moved_count,
                closure,
                &closure_count
            );

            int existing = -1;

            for (int s = 0; s < dfa->state_count; s++)
            {
                int stored_count = 0;

                while (stored_count < TA_MAX_NFA_STATES &&
                       dfa->subset[s][stored_count] != -1)
                {
                    stored_count++;
                }

                if (same_subset(
                        dfa->subset[s],
                        stored_count,
                        closure,
                        closure_count))
                {
                    existing = s;
                    break;
                }
            }

            if (existing == -1)
            {
                if (dfa->state_count >= TA_MAX_DFA_STATES)
                    continue;

                existing = dfa->state_count++;

                for (int i = 0; i < TA_MAX_NFA_STATES; i++)
                    dfa->subset[existing][i] = -1;

                for (int i = 0; i < closure_count; i++)
                    dfa->subset[existing][i] = closure[i];

                determine_dfa_token(
                    nfa,
                    dfa->subset[existing],
                    &dfa->accepting[existing],
                    &dfa->token_type[existing],
                    &dfa->priority[existing]
                );
            }

            dfa->transition[current][symbol] = existing;
        }

        current++;
    }
}


/* ============================================================
   DFA MINIMIZATION
   ============================================================ */

void minimize_token_dfa(
    const TADFA *dfa,
    TAMinDFA *min_dfa)
{
    memset(min_dfa, 0, sizeof(TAMinDFA));

    int group[TA_MAX_DFA_STATES];

    /*
       Initially separate accepting and non-accepting states.
    */

    for (int i = 0; i < dfa->state_count; i++)
    {
        if (dfa->accepting[i])
            group[i] = 1;
        else
            group[i] = 0;
    }

    int group_count = 2;

    int changed = 1;

    while (changed)
    {
        changed = 0;

        int new_group[TA_MAX_DFA_STATES];
        int next_group = 0;

        for (int i = 0; i < dfa->state_count; i++)
        {
            int assigned = -1;

            for (int j = 0; j < i; j++)
            {
                int same = 1;

                /*
                   Accepting status and token type must match.
                */

                if (dfa->accepting[i] != dfa->accepting[j])
                {
                    same = 0;
                }
                else if (dfa->accepting[i] &&
                         dfa->token_type[i] != dfa->token_type[j])
                {
                    same = 0;
                }

                /*
                   Check every input symbol.
                */

                if (same)
                {
                    for (int symbol = 0;
                         symbol < TA_MAX_ALPHABET;
                         symbol++)
                    {
                        int ti = dfa->transition[i][symbol];
                        int tj = dfa->transition[j][symbol];

                        int gi = (ti == -1) ? -1 : group[ti];
                        int gj = (tj == -1) ? -1 : group[tj];

                        if (gi != gj)
                        {
                            same = 0;
                            break;
                        }
                    }
                }

                if (same)
                {
                    assigned = new_group[j];
                    break;
                }
            }

            if (assigned == -1)
            {
                assigned = next_group++;
            }

            new_group[i] = assigned;
        }

        if (next_group != group_count)
            changed = 1;

        group_count = next_group;

        for (int i = 0; i < dfa->state_count; i++)
            group[i] = new_group[i];
    }

    min_dfa->state_count = group_count;

    min_dfa->start_state =
        group[dfa->start_state];

    /*
       Build minimized transitions.
    */

    for (int g = 0; g < group_count; g++)
    {
        int representative = -1;

        for (int s = 0; s < dfa->state_count; s++)
        {
            if (group[s] == g)
            {
                representative = s;
                break;
            }
        }

        if (representative == -1)
            continue;

        min_dfa->accepting[g] =
            dfa->accepting[representative];

        min_dfa->token_type[g] =
            dfa->token_type[representative];

        min_dfa->priority[g] =
            dfa->priority[representative];

        for (int symbol = 0;
             symbol < TA_MAX_ALPHABET;
             symbol++)
        {
            int target =
                dfa->transition[representative][symbol];

            if (target == -1)
                min_dfa->transition[g][symbol] = -1;
            else
                min_dfa->transition[g][symbol] =
                    group[target];
        }
    }
}


/* ============================================================
   TOKEN NAMES
   ============================================================ */

const char *token_type_name(TATokenType type)
{
    switch (type)
    {
        case TA_TOKEN_INT:
            return "KEYWORD_INT";

        case TA_TOKEN_FLOAT:
            return "KEYWORD_FLOAT";

        case TA_TOKEN_IDENTIFIER:
            return "IDENTIFIER";

        case TA_TOKEN_NUMBER:
            return "NUMBER";

        case TA_TOKEN_PLUS:
            return "PLUS";

        case TA_TOKEN_MINUS:
            return "MINUS";

        case TA_TOKEN_MULTIPLY:
            return "MULTIPLY";

        case TA_TOKEN_DIVIDE:
            return "DIVIDE";

        case TA_TOKEN_ASSIGN:
            return "ASSIGN";

        case TA_TOKEN_LPAREN:
            return "LPAREN";

        case TA_TOKEN_RPAREN:
            return "RPAREN";

        case TA_TOKEN_SEMICOLON:
            return "SEMICOLON";

        default:
            return "NONE";
    }
}


/* ============================================================
   PRINT NFA
   ============================================================ */

void print_token_nfa(const TANFA *nfa)
{
    printf("\nNFA States: %d\n", nfa->state_count);
    printf("Start State: %d\n\n", nfa->start_state);

    printf("%-8s %-10s %-8s\n",
           "FROM",
           "INPUT",
           "TO");

    printf("--------------------------------\n");

    for (int i = 0;
         i < nfa->transition_count;
         i++)
    {
        TANFATransition t =
            nfa->transitions[i];

        if (t.symbol == TA_EPSILON)
        {
            printf("%-8d %-10s %-8d\n",
                   t.from,
                   "epsilon",
                   t.to);
        }
        else if (isprint(t.symbol))
        {
            printf("%-8d %-10c %-8d\n",
                   t.from,
                   (char)t.symbol,
                   t.to);
        }
    }

    printf("\nAccepting States:\n");

    for (int i = 0;
         i < nfa->state_count;
         i++)
    {
        if (nfa->accepting[i])
        {
            printf(
                "State %d -> %s (priority %d)\n",
                i,
                token_type_name(
                    nfa->token_type[i]),
                nfa->priority[i]
            );
        }
    }
}


/* ============================================================
   PRINT DFA
   ============================================================ */

void print_token_dfa(const TADFA *dfa)
{
    printf("\nDFA States: %d\n", dfa->state_count);
    printf("Start State: %d\n\n", dfa->start_state);

    for (int state = 0;
         state < dfa->state_count;
         state++)
    {
        printf("State %d", state);

        if (dfa->accepting[state])
        {
            printf(
                " -> ACCEPT (%s)",
                token_type_name(
                    dfa->token_type[state])
            );
        }

        printf("\n");

        for (int symbol = 0;
             symbol < TA_MAX_ALPHABET;
             symbol++)
        {
            int target =
                dfa->transition[state][symbol];

            if (target != -1 &&
                isprint(symbol))
            {
                printf(
                    "    '%c' -> %d\n",
                    (char)symbol,
                    target
                );
            }
        }
    }
}


/* ============================================================
   PRINT MINIMIZED DFA
   ============================================================ */

void print_min_token_dfa(const TAMinDFA *dfa)
{
    printf(
        "\nMinimized DFA States: %d\n",
        dfa->state_count
    );

    printf(
        "Start State: %d\n\n",
        dfa->start_state
    );

    for (int state = 0;
         state < dfa->state_count;
         state++)
    {
        printf("State %d", state);

        if (dfa->accepting[state])
        {
            printf(
                " -> ACCEPT (%s)",
                token_type_name(
                    dfa->token_type[state])
            );
        }

        printf("\n");

        for (int symbol = 0;
             symbol < TA_MAX_ALPHABET;
             symbol++)
        {
            int target =
                dfa->transition[state][symbol];

            if (target != -1 &&
                isprint(symbol))
            {
                printf(
                    "    '%c' -> %d\n",
                    (char)symbol,
                    target
                );
            }
        }
    }
}


/* ============================================================
   RECOGNIZE TOKEN
   ============================================================ */

TATokenType recognize_token(
    const TAMinDFA *dfa,
    const char *input)
{
    int state = dfa->start_state;

    for (int i = 0; input[i] != '\0'; i++)
    {
        unsigned char ch =
            (unsigned char)input[i];

        if (ch >= TA_MAX_ALPHABET)
            return TA_TOKEN_UNKNOWN;

        int next =
            dfa->transition[state][ch];

        if (next == -1)
            return TA_TOKEN_UNKNOWN;

        state = next;
    }

    if (dfa->accepting[state])
        return dfa->token_type[state];

    return TA_TOKEN_UNKNOWN;
}