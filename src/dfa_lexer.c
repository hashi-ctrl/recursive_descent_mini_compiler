#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "dfa_lexer.h"
#include "token_automata.h"


/*
    The minimized DFA is constructed once
    and then reused by the lexer.
*/

static TAMinDFA lexerDFA;

static int lexerInitialized = 0;


/* ============================================================
   TOKEN CONVERSION
   ============================================================ */

static TokenType convertTokenType(TATokenType type)
{
    switch (type)
    {
        case TA_TOKEN_INT:
            return TOKEN_INT;

        case TA_TOKEN_FLOAT:
            return TOKEN_FLOAT;

        case TA_TOKEN_IDENTIFIER:
            return TOKEN_IDENTIFIER;

        case TA_TOKEN_NUMBER:
            return TOKEN_NUMBER;

        case TA_TOKEN_PLUS:
            return TOKEN_PLUS;

        case TA_TOKEN_MINUS:
            return TOKEN_MINUS;

        case TA_TOKEN_MULTIPLY:
            return TOKEN_MULTIPLY;

        case TA_TOKEN_DIVIDE:
            return TOKEN_DIVIDE;

        case TA_TOKEN_ASSIGN:
            return TOKEN_ASSIGN;

        case TA_TOKEN_LPAREN:
            return TOKEN_LPAREN;

        case TA_TOKEN_RPAREN:
            return TOKEN_RPAREN;

        case TA_TOKEN_SEMICOLON:
            return TOKEN_SEMICOLON;

        default:
            return TOKEN_UNKNOWN;
    }
}


/* ============================================================
   INITIALIZE DFA
   ============================================================ */

void dfaLexerInit(void)
{
    TANFA nfa;
    TADFA dfa;

    /*
        Step 1:
        Build combined token NFA.
    */

    build_token_nfa(&nfa);


    /*
        Step 2:
        Convert NFA -> DFA.
    */

    token_nfa_to_dfa(&nfa, &dfa);


    /*
        Step 3:
        Minimize DFA.
    */

    minimize_token_dfa(&dfa, &lexerDFA);

    lexerInitialized = 1;
}


/* ============================================================
   GET NEXT TOKEN
   ============================================================ */

Token dfaGetNextToken(
    const char *input,
    int *position)
{
    Token token;

    token.type = TOKEN_UNKNOWN;
    token.lexeme[0] = '\0';


    /*
        Make sure DFA exists.
    */

    if (!lexerInitialized)
        dfaLexerInit();


    /*
        Skip whitespace.
    */

    while (input[*position] != '\0' &&
           isspace((unsigned char)input[*position]))
    {
        (*position)++;
    }


    /*
        End of source.
    */

    if (input[*position] == '\0')
    {
        token.type = TOKEN_EOF;
        strcpy(token.lexeme, "EOF");

        return token;
    }


    /*
        Start DFA traversal.
    */

    int state = lexerDFA.start_state;

    int current = *position;

    int lastAcceptPosition = -1;

    TATokenType lastTokenType =
        TA_TOKEN_UNKNOWN;


    /*
        MAXIMAL MUNCH

        Continue reading characters as long
        as the DFA has a valid transition.

        Remember the LAST accepting state.
    */

    while (input[current] != '\0')
    {
        unsigned char ch =
            (unsigned char)input[current];

        if (ch >= TA_MAX_ALPHABET)
            break;


        int next =
            lexerDFA.transition[state][ch];


        /*
            No transition:
            stop scanning this lexeme.
        */

        if (next == -1)
            break;


        state = next;
        current++;


        /*
            If this state accepts a token,
            remember it.

            We DO NOT stop immediately.

            This is what gives us longest match.
        */

        if (lexerDFA.accepting[state])
        {
            lastAcceptPosition = current;

            lastTokenType =
                lexerDFA.token_type[state];
        }
    }


    /*
        No valid token found.
    */

    if (lastAcceptPosition == -1)
    {
        token.type = TOKEN_UNKNOWN;

        token.lexeme[0] =
            input[*position];

        token.lexeme[1] =
            '\0';

        (*position)++;

        return token;
    }


    /*
        Copy longest accepted lexeme.
    */

    int length =
        lastAcceptPosition - *position;


    /*
        Prevent buffer overflow.
    */

    if (length >= DFA_LEXEM_SIZE)
        length = DFA_LEXEM_SIZE - 1;


    strncpy(
        token.lexeme,
        input + *position,
        length
    );

    token.lexeme[length] = '\0';


    /*
        Convert automata token type
        to the compiler's TokenType.
    */

    token.type =
        convertTokenType(lastTokenType);


    /*
        Advance source position
        to the end of the recognized token.
    */

    *position = lastAcceptPosition;


    return token;
}