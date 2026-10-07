#include "automata.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

/* ============================================================
   NFA UTILITIES
   ============================================================ */

static void init_nfa(NFA *nfa)
{
    nfa->state_count = 0;
    nfa->start_state = -1;
    nfa->accept_state = -1;
    nfa->transition_count = 0;
}

static int new_nfa_state(NFA *nfa)
{
    if (nfa->state_count >= MAX_NFA_STATES)
        return -1;

    return nfa->state_count++;
}

static void add_nfa_transition(NFA *nfa, int from, int symbol, int to)
{
    if (nfa->transition_count >= 1024)
        return;

    nfa->transitions[nfa->transition_count].from = from;
    nfa->transitions[nfa->transition_count].symbol = symbol;
    nfa->transitions[nfa->transition_count].to = to;

    nfa->transition_count++;
}

/* ============================================================
   REGEX PARSER
   Supports:
       |   union
       *   zero or more
       +   one or more
       ?   optional
       ()  grouping
       \x  escaped literal
       ordinary characters
   ============================================================ */

typedef struct {
    int start;
    int end;
} Fragment;

typedef struct {
    const char *regex;
    int pos;
    NFA *nfa;
} RegexParser;

static Fragment parse_expression(RegexParser *p);

static char current_char(RegexParser *p)
{
    return p->regex[p->pos];
}

static char consume_char(RegexParser *p)
{
    return p->regex[p->pos++];
}

static Fragment make_literal(RegexParser *p, char c)
{
    Fragment f;

    f.start = new_nfa_state(p->nfa);
    f.end = new_nfa_state(p->nfa);

    add_nfa_transition(p->nfa, f.start, (unsigned char)c, f.end);

    return f;
}

static Fragment make_epsilon(RegexParser *p)
{
    Fragment f;

    f.start = new_nfa_state(p->nfa);
    f.end = new_nfa_state(p->nfa);

    add_nfa_transition(p->nfa, f.start, EPSILON, f.end);

    return f;
}

static Fragment concatenate(NFA *nfa, Fragment a, Fragment b)
{
    Fragment result;

    add_nfa_transition(nfa, a.end, EPSILON, b.start);

    result.start = a.start;
    result.end = b.end;

    return result;
}

static Fragment alternate(NFA *nfa, Fragment a, Fragment b)
{
    Fragment result;

    result.start = new_nfa_state(nfa);
    result.end = new_nfa_state(nfa);

    add_nfa_transition(nfa, result.start, EPSILON, a.start);
    add_nfa_transition(nfa, result.start, EPSILON, b.start);

    add_nfa_transition(nfa, a.end, EPSILON, result.end);
    add_nfa_transition(nfa, b.end, EPSILON, result.end);

    return result;
}

static Fragment star(NFA *nfa, Fragment a)
{
    Fragment result;

    result.start = new_nfa_state(nfa);
    result.end = new_nfa_state(nfa);

    add_nfa_transition(nfa, result.start, EPSILON, a.start);
    add_nfa_transition(nfa, result.start, EPSILON, result.end);

    add_nfa_transition(nfa, a.end, EPSILON, a.start);
    add_nfa_transition(nfa, a.end, EPSILON, result.end);

    return result;
}

static Fragment plus(NFA *nfa, Fragment a)
{
    Fragment result;

    result.start = new_nfa_state(nfa);
    result.end = new_nfa_state(nfa);

    add_nfa_transition(nfa, result.start, EPSILON, a.start);

    add_nfa_transition(nfa, a.end, EPSILON, a.start);
    add_nfa_transition(nfa, a.end, EPSILON, result.end);

    return result;
}

static Fragment optional_fragment(NFA *nfa, Fragment a)
{
    Fragment result;

    result.start = new_nfa_state(nfa);
    result.end = new_nfa_state(nfa);

    add_nfa_transition(nfa, result.start, EPSILON, a.start);
    add_nfa_transition(nfa, result.start, EPSILON, result.end);

    add_nfa_transition(nfa, a.end, EPSILON, result.end);

    return result;
}

/*
    factor:
        literal
        '(' expression ')'
        factor*
        factor+
        factor?
*/
static Fragment parse_factor(RegexParser *p)
{
    Fragment f;

    if (current_char(p) == '\0')
        return make_epsilon(p);

    if (current_char(p) == '(')
    {
        consume_char(p);

        f = parse_expression(p);

        if (current_char(p) == ')')
            consume_char(p);
    }
    else if (current_char(p) == '\\')
    {
        consume_char(p);

        if (current_char(p) != '\0')
        {
            char c = consume_char(p);
            f = make_literal(p, c);
        }
        else
        {
            f = make_epsilon(p);
        }
    }
    else
    {
        char c = consume_char(p);
        f = make_literal(p, c);
    }

    while (current_char(p) == '*' ||
           current_char(p) == '+' ||
           current_char(p) == '?')
    {
        char op = consume_char(p);

        if (op == '*')
            f = star(p->nfa, f);
        else if (op == '+')
            f = plus(p->nfa, f);
        else if (op == '?')
            f = optional_fragment(p->nfa, f);
    }

    return f;
}

/*
    term:
        factor factor factor ...
*/
static Fragment parse_term(RegexParser *p)
{
    Fragment result;
    int first = 1;

    while (current_char(p) != '\0' &&
           current_char(p) != ')' &&
           current_char(p) != '|')
    {
        Fragment f = parse_factor(p);

        if (first)
        {
            result = f;
            first = 0;
        }
        else
        {
            result = concatenate(p->nfa, result, f);
        }
    }

    if (first)
        result = make_epsilon(p);

    return result;
}

/*
    expression:
        term
        term | term
*/
static Fragment parse_expression(RegexParser *p)
{
    Fragment result = parse_term(p);

    while (current_char(p) == '|')
    {
        consume_char(p);

        Fragment right = parse_term(p);

        result = alternate(p->nfa, result, right);
    }

    return result;
}

int regex_to_nfa(const char *regex, NFA *nfa)
{
    RegexParser parser;
    Fragment result;

    init_nfa(nfa);

    parser.regex = regex;
    parser.pos = 0;
    parser.nfa = nfa;

    result = parse_expression(&parser);

    nfa->start_state = result.start;
    nfa->accept_state = result.end;

    return 1;
}

/* ============================================================
   EPSILON CLOSURE
   ============================================================ */

static void epsilon_closure(
    const NFA *nfa,
    unsigned char *set)
{
    int changed = 1;

    while (changed)
    {
        changed = 0;

        for (int i = 0; i < nfa->transition_count; i++)
        {
            int from = nfa->transitions[i].from;
            int symbol = nfa->transitions[i].symbol;
            int to = nfa->transitions[i].to;

            if (symbol == EPSILON &&
                set[from] &&
                !set[to])
            {
                set[to] = 1;
                changed = 1;
            }
        }
    }
}

/* ============================================================
   MOVE
   ============================================================ */

static void move_set(
    const NFA *nfa,
    const unsigned char *set,
    unsigned char *result,
    int symbol)
{
    memset(result, 0, MAX_NFA_STATES);

    for (int i = 0; i < nfa->transition_count; i++)
    {
        int from = nfa->transitions[i].from;
        int trans_symbol = nfa->transitions[i].symbol;
        int to = nfa->transitions[i].to;

        if (trans_symbol == symbol && set[from])
            result[to] = 1;
    }
}

/* ============================================================
   DFA STATE COMPARISON
   ============================================================ */

static int same_subset(
    const unsigned char *a,
    const unsigned char *b)
{
    return memcmp(a, b, MAX_NFA_STATES) == 0;
}

static int find_dfa_state(
    const DFA *dfa,
    const unsigned char *subset)
{
    for (int i = 0; i < dfa->state_count; i++)
    {
        if (same_subset(dfa->subset[i], subset))
            return i;
    }

    return -1;
}

/* ============================================================
   NFA -> DFA
   SUBSET CONSTRUCTION
   ============================================================ */

int nfa_to_dfa(const NFA *nfa, DFA *dfa)
{
    unsigned char start[MAX_NFA_STATES];

    memset(dfa, 0, sizeof(DFA));

    for (int i = 0; i < MAX_DFA_STATES; i++)
    {
        for (int c = 0; c < 128; c++)
            dfa->transition[i][c] = -1;
    }

    memset(start, 0, sizeof(start));

    start[nfa->start_state] = 1;
    epsilon_closure(nfa, start);

    memcpy(dfa->subset[0], start, MAX_NFA_STATES);

    dfa->state_count = 1;
    dfa->start_state = 0;

    int current = 0;

    while (current < dfa->state_count)
    {
        unsigned char current_set[MAX_NFA_STATES];

        memcpy(
            current_set,
            dfa->subset[current],
            MAX_NFA_STATES);

        if (current_set[nfa->accept_state])
            dfa->accepting[current] = 1;

        for (int symbol = 0; symbol < 128; symbol++)
        {
            unsigned char moved[MAX_NFA_STATES];
            unsigned char closed[MAX_NFA_STATES];

            move_set(
                nfa,
                current_set,
                moved,
                symbol);

            int has_state = 0;

            for (int i = 0; i < MAX_NFA_STATES; i++)
            {
                if (moved[i])
                {
                    has_state = 1;
                    break;
                }
            }

            if (!has_state)
                continue;

            memcpy(closed, moved, MAX_NFA_STATES);
            epsilon_closure(nfa, closed);

            int existing = find_dfa_state(dfa, closed);

            if (existing == -1)
            {
                if (dfa->state_count >= MAX_DFA_STATES)
                    return 0;

                existing = dfa->state_count++;

                memcpy(
                    dfa->subset[existing],
                    closed,
                    MAX_NFA_STATES);

                if (closed[nfa->accept_state])
                    dfa->accepting[existing] = 1;
            }

            dfa->transition[current][symbol] = existing;
        }

        current++;
    }

    return 1;
}

/* ============================================================
   DFA MINIMIZATION
   PARTITION REFINEMENT
   ============================================================ */

int minimize_dfa(const DFA *dfa, MinDFA *min)
{
    int group[MAX_DFA_STATES];
    int new_group[MAX_DFA_STATES];

    int group_count = 0;

    memset(min, 0, sizeof(MinDFA));

    /*
        Initial partition:
        accepting states vs non-accepting states
    */

    for (int i = 0; i < dfa->state_count; i++)
    {
        group[i] = dfa->accepting[i] ? 1 : 0;
    }

    group_count = 2;

    int changed = 1;

    while (changed)
    {
        changed = 0;

        int next_group = 0;

        for (int i = 0; i < dfa->state_count; i++)
            new_group[i] = -1;

        for (int i = 0; i < dfa->state_count; i++)
        {
            if (new_group[i] != -1)
                continue;

            new_group[i] = next_group;

            for (int j = i + 1; j < dfa->state_count; j++)
            {
                if (new_group[j] != -1)
                    continue;

                if (group[i] != group[j])
                    continue;

                int equivalent = 1;

                for (int c = 0; c < 128; c++)
                {
                    int a = dfa->transition[i][c];
                    int b = dfa->transition[j][c];

                    int ga = (a == -1) ? -1 : group[a];
                    int gb = (b == -1) ? -1 : group[b];

                    if (ga != gb)
                    {
                        equivalent = 0;
                        break;
                    }
                }

                if (equivalent)
                    new_group[j] = next_group;
            }

            next_group++;
        }

        if (next_group != group_count)
            changed = 1;

        group_count = next_group;

        memcpy(
            group,
            new_group,
            sizeof(group));
    }

    min->state_count = group_count;
    min->start_state = group[dfa->start_state];

    for (int g = 0; g < group_count; g++)
    {
        int representative = -1;

        for (int i = 0; i < dfa->state_count; i++)
        {
            if (group[i] == g)
            {
                representative = i;
                break;
            }
        }

        if (representative == -1)
            continue;

        min->accepting[g] = dfa->accepting[representative];

        for (int c = 0; c < 128; c++)
        {
            int target = dfa->transition[representative][c];

            if (target != -1)
                min->transition[g][c] = group[target];
            else
                min->transition[g][c] = -1;
        }
    }

    return 1;
}

/* ============================================================
   PRINT NFA
   ============================================================ */

void print_nfa(const NFA *nfa)
{
    printf("\n========== ε-NFA ==========\n");

    printf("States: %d\n", nfa->state_count);
    printf("Start : q%d\n", nfa->start_state);
    printf("Final : q%d\n", nfa->accept_state);

    printf("\nTransitions:\n");

    for (int i = 0; i < nfa->transition_count; i++)
    {
        int symbol = nfa->transitions[i].symbol;

        printf(
            "q%d -- ",
            nfa->transitions[i].from);

        if (symbol == EPSILON)
            printf("ε");
        else
            printf("%c", symbol);

        printf(
            " --> q%d\n",
            nfa->transitions[i].to);
    }
}

/* ============================================================
   PRINT DFA
   ============================================================ */

void print_dfa(const DFA *dfa)
{
    printf("\n========== DFA ==========\n");

    printf("States: %d\n", dfa->state_count);
    printf("Start : D%d\n", dfa->start_state);

    printf("\nAccepting states: ");

    for (int i = 0; i < dfa->state_count; i++)
    {
        if (dfa->accepting[i])
            printf("D%d ", i);
    }

    printf("\n\nTransitions:\n");

    for (int state = 0;
         state < dfa->state_count;
         state++)
    {
        for (int c = 0; c < 128; c++)
        {
            int target = dfa->transition[state][c];

            if (target != -1 &&
                isprint(c))
            {
                printf(
                    "D%d -- %c --> D%d\n",
                    state,
                    c,
                    target);
            }
        }
    }
}

/* ============================================================
   PRINT MINIMIZED DFA
   ============================================================ */

void print_min_dfa(const MinDFA *dfa)
{
    printf("\n========== MINIMIZED DFA ==========\n");

    printf("States: %d\n", dfa->state_count);
    printf("Start : M%d\n", dfa->start_state);

    printf("\nAccepting states: ");

    for (int i = 0; i < dfa->state_count; i++)
    {
        if (dfa->accepting[i])
            printf("M%d ", i);
    }

    printf("\n\nTransitions:\n");

    for (int state = 0;
         state < dfa->state_count;
         state++)
    {
        for (int c = 0; c < 128; c++)
        {
            int target = dfa->transition[state][c];

            if (target != -1 &&
                isprint(c))
            {
                printf(
                    "M%d -- %c --> M%d\n",
                    state,
                    c,
                    target);
            }
        }
    }
}

/* ============================================================
   DFA EXECUTION
   ============================================================ */

int dfa_accepts(const DFA *dfa, const char *input)
{
    int state = dfa->start_state;

    for (int i = 0; input[i] != '\0'; i++)
    {
        unsigned char c = input[i];

        if (c >= 128)
            return 0;

        state = dfa->transition[state][c];

        if (state == -1)
            return 0;
    }

    return dfa->accepting[state];
}

int min_dfa_accepts(const MinDFA *dfa, const char *input)
{
    int state = dfa->start_state;

    for (int i = 0; input[i] != '\0'; i++)
    {
        unsigned char c = input[i];

        if (c >= 128)
            return 0;

        state = dfa->transition[state][c];

        if (state == -1)
            return 0;
    }

    return dfa->accepting[state];
}