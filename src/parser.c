#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "parser.h"
#include "lexer.h"
#include "symbol_table.h"


/* ---------- Symbol Table ---------- */

static SymbolTable symbol_table;


/* ---------- TAC Buffer ---------- */

#define MAX_TAC_LINES 500
#define MAX_TAC_LENGTH 128

static char tac_code[MAX_TAC_LINES][MAX_TAC_LENGTH];
static int tac_count = 0;


/* ---------- Helper structures ---------- */

typedef struct {
    char value[64];
    SymbolType type;
    int valid;
} ExprResult;


/* ---------- Forward declarations ---------- */

static void advance(Parser *parser);
static void syntax_error(Parser *parser, const char *message);
static void semantic_error(Parser *parser, const char *message);
static void expect(Parser *parser, TokenType type);

static void parse_statement(Parser *parser);
static void parse_declaration(Parser *parser);
static void parse_assignment(Parser *parser);

static ExprResult parse_expression(Parser *parser);
static ExprResult parse_term(Parser *parser);
static ExprResult parse_factor(Parser *parser);

static ExprResult make_result(const char *value,
                              SymbolType type);

static ExprResult make_temp(Parser *parser,
                            SymbolType type);

static void emit_tac(const char *format, ...);
static void print_tac(void);


/* ---------- Parser initialization ---------- */

void parser_init(Parser *parser, const char *source)
{
    parser->source = source;
    parser->position = 0;
    parser->error_count = 0;
    parser->temp_count = 0;

    /* Initialize symbol table */
    initSymbolTable(&symbol_table);

    /* Clear TAC buffer */
    tac_count = 0;

    parser->current_token =
        getNextToken(parser->source, &parser->position);
}


/* ---------- TAC handling ---------- */

static void emit_tac(const char *format, ...)
{
    va_list args;

    if (tac_count >= MAX_TAC_LINES)
        return;

    va_start(args, format);

    vsnprintf(tac_code[tac_count],
              MAX_TAC_LENGTH,
              format,
              args);

    va_end(args);

    tac_count++;
}


static void print_tac(void)
{
    for (int i = 0; i < tac_count; i++)
    {
        printf("%s\n", tac_code[i]);
    }
}


/* ---------- Token handling ---------- */

static void advance(Parser *parser)
{
    parser->current_token =
        getNextToken(parser->source, &parser->position);
}


static void syntax_error(Parser *parser,
                         const char *message)
{
    printf("Syntax Error: %s", message);

    if (parser->current_token.lexeme[0] != '\0')
    {
        printf(" (near '%s')",
               parser->current_token.lexeme);
    }

    printf("\n");

    parser->error_count++;
}


static void semantic_error(Parser *parser,
                           const char *message)
{
    printf("Semantic Error: %s\n",
           message);

    parser->error_count++;
}


static void expect(Parser *parser,
                   TokenType type)
{
    if (parser->current_token.type == type)
    {
        advance(parser);
    }
    else
    {
        syntax_error(parser,
                     "Unexpected token");
    }
}


/* ---------- Expression helpers ---------- */

static ExprResult make_result(const char *value,
                              SymbolType type)
{
    ExprResult result;

    strncpy(result.value,
            value,
            sizeof(result.value) - 1);

    result.value[sizeof(result.value) - 1] = '\0';

    result.type = type;
    result.valid = 1;

    return result;
}


static ExprResult make_temp(Parser *parser,
                            SymbolType type)
{
    ExprResult result;

    parser->temp_count++;

    snprintf(result.value,
             sizeof(result.value),
             "t%d",
             parser->temp_count);

    result.type = type;
    result.valid = 1;

    return result;
}


/* ---------- Program ---------- */

void parse_program(Parser *parser)
{
    while (parser->current_token.type != TOKEN_EOF)
    {
        parse_statement(parser);

        /*
         * Prevent infinite loops if a syntax error occurs
         * and the parser cannot make progress.
         */
        if (parser->current_token.type == TOKEN_UNKNOWN)
        {
            advance(parser);
        }
    }


    /*
     * Generate/display TAC only when the entire
     * program is free from syntax and semantic errors.
     */
    if (parser->error_count == 0)
    {
        print_tac();
    }
    else
    {
        printf("No code generated due to errors.\n");
    }


    /*
     * Display symbol table after parsing.
     */
    printSymbolTable(&symbol_table);
}


/* ---------- Statements ---------- */

static void parse_statement(Parser *parser)
{
    if (parser->current_token.type == TOKEN_INT ||
        parser->current_token.type == TOKEN_FLOAT)
    {
        parse_declaration(parser);
    }
    else if (parser->current_token.type == TOKEN_IDENTIFIER)
    {
        parse_assignment(parser);
    }
    else
    {
        syntax_error(parser,
                     "Expected declaration or assignment");

        /*
         * Skip the problematic token so parsing can continue.
         */
        advance(parser);
    }
}


/* ---------- Declaration ---------- */

static void parse_declaration(Parser *parser)
{
    TokenType declaration_type =
        parser->current_token.type;


    /* Consume int / float */
    advance(parser);


    if (parser->current_token.type != TOKEN_IDENTIFIER)
    {
        syntax_error(parser,
                     "Expected identifier after type");

        return;
    }


    /*
     * Save identifier name before consuming it.
     */
    char identifier[64];

    strcpy(identifier,
           parser->current_token.lexeme);


    /*
     * Convert parser token type into
     * symbol table type.
     */
    SymbolType symbol_type;

    if (declaration_type == TOKEN_INT)
    {
        symbol_type = SYMBOL_INT;
    }
    else
    {
        symbol_type = SYMBOL_FLOAT;
    }


    /*
     * Insert identifier into symbol table.
     */
    if (!insertSymbol(&symbol_table,
                      identifier,
                      symbol_type,
                      "global"))
    {
        char message[128];

        snprintf(message,
                 sizeof(message),
                 "Duplicate declaration '%s'",
                 identifier);

        semantic_error(parser,
                       message);
    }


    /* Consume identifier */
    advance(parser);


    /* Expect semicolon */
    expect(parser,
           TOKEN_SEMICOLON);
}


/* ---------- Assignment ---------- */

static void parse_assignment(Parser *parser)
{
    char identifier[64];

    strcpy(identifier,
           parser->current_token.lexeme);


    /*
     * Check whether the left-hand-side identifier
     * has been declared.
     */
    Symbol *symbol =
        lookupSymbol(&symbol_table,
                     identifier);


    if (symbol == NULL)
    {
        char message[128];

        snprintf(message,
                 sizeof(message),
                 "Undeclared identifier '%s'",
                 identifier);

        semantic_error(parser,
                       message);
    }


    /* Consume identifier */
    expect(parser,
           TOKEN_IDENTIFIER);


    /* Expect '=' */
    if (parser->current_token.type != TOKEN_ASSIGN)
    {
        syntax_error(parser,
                     "Expected '=' after identifier");

        return;
    }

    advance(parser);


    /* Parse expression */
    ExprResult expression =
        parse_expression(parser);


    /* Expect semicolon */
    if (parser->current_token.type != TOKEN_SEMICOLON)
    {
        syntax_error(parser,
                     "Expected ';' after expression");

        return;
    }

    advance(parser);


    /*
     * Type checking:
     *
     * int <- int       : VALID
     * float <- int     : VALID
     * float <- float   : VALID
     * int <- float     : ERROR
     */
    if (symbol != NULL &&
        expression.valid)
    {
        if (symbol->type == SYMBOL_INT &&
            expression.type == SYMBOL_FLOAT)
        {
            char message[160];

            snprintf(message,
                     sizeof(message),
                     "Cannot assign float expression to int variable '%s'",
                     identifier);

            semantic_error(parser,
                           message);
        }
        else
        {
            /*
             * Assignment is valid,
             * so mark variable as initialized.
             */
            markInitialized(&symbol_table,
                            identifier);
        }
    }


    /*
     * Store final assignment in TAC buffer.
     *
     * It will only be printed if the complete
     * program has no errors.
     */
    emit_tac("%s = %s",
             identifier,
             expression.value);
}


/* ---------- Expression ---------- */

/*
 * expression → term { (+ | -) term }
 */

static ExprResult parse_expression(Parser *parser)
{
    ExprResult left =
        parse_term(parser);


    while (parser->current_token.type == TOKEN_PLUS ||
           parser->current_token.type == TOKEN_MINUS)
    {
        TokenType operator =
            parser->current_token.type;

        advance(parser);


        ExprResult right =
            parse_term(parser);


        /*
         * Type promotion:
         *
         * int + int       -> int
         * int + float     -> float
         * float + int     -> float
         * float + float   -> float
         */
        SymbolType result_type;

        if (left.type == SYMBOL_FLOAT ||
            right.type == SYMBOL_FLOAT)
        {
            result_type = SYMBOL_FLOAT;
        }
        else
        {
            result_type = SYMBOL_INT;
        }


        ExprResult temp =
            make_temp(parser,
                      result_type);


        if (operator == TOKEN_PLUS)
        {
            emit_tac("%s = %s + %s",
                     temp.value,
                     left.value,
                     right.value);
        }
        else
        {
            emit_tac("%s = %s - %s",
                     temp.value,
                     left.value,
                     right.value);
        }


        /*
         * Expression is valid only if both
         * operands are valid.
         */
        temp.valid =
            left.valid && right.valid;


        left = temp;
    }


    return left;
}


/* ---------- Term ---------- */

/*
 * term → factor { (* | /) factor }
 */

static ExprResult parse_term(Parser *parser)
{
    ExprResult left =
        parse_factor(parser);


    while (parser->current_token.type == TOKEN_MULTIPLY ||
           parser->current_token.type == TOKEN_DIVIDE)
    {
        TokenType operator =
            parser->current_token.type;

        advance(parser);


        ExprResult right =
            parse_factor(parser);


        /*
         * Type promotion:
         *
         * int * int       -> int
         * int * float     -> float
         * float * int     -> float
         * float * float   -> float
         */
        SymbolType result_type;

        if (left.type == SYMBOL_FLOAT ||
            right.type == SYMBOL_FLOAT)
        {
            result_type = SYMBOL_FLOAT;
        }
        else
        {
            result_type = SYMBOL_INT;
        }


        ExprResult temp =
            make_temp(parser,
                      result_type);


        if (operator == TOKEN_MULTIPLY)
        {
            emit_tac("%s = %s * %s",
                     temp.value,
                     left.value,
                     right.value);
        }
        else
        {
            emit_tac("%s = %s / %s",
                     temp.value,
                     left.value,
                     right.value);
        }


        /*
         * Expression is valid only if both
         * operands are valid.
         */
        temp.valid =
            left.valid && right.valid;


        left = temp;
    }


    return left;
}


/* ---------- Factor ---------- */

/*
 * factor → identifier
 *        | number
 *        | '(' expression ')'
 */

static ExprResult parse_factor(Parser *parser)
{
    /*
     * ---------- Identifier ----------
     */
    if (parser->current_token.type == TOKEN_IDENTIFIER)
    {
        char identifier[64];

        strcpy(identifier,
               parser->current_token.lexeme);


        /*
         * Look up identifier in symbol table.
         */
        Symbol *symbol =
            lookupSymbol(&symbol_table,
                         identifier);


        /*
         * Undeclared identifier.
         */
        if (symbol == NULL)
        {
            char message[128];

            snprintf(message,
                     sizeof(message),
                     "Undeclared identifier '%s'",
                     identifier);

            semantic_error(parser,
                           message);

            advance(parser);


            /*
             * Return invalid expression result.
             */
            ExprResult result =
                make_result(identifier,
                            SYMBOL_INT);

            result.valid = 0;

            return result;
        }


        /*
         * Identifier gets its type from
         * the symbol table.
         */
        ExprResult result =
            make_result(identifier,
                        symbol->type);


        advance(parser);

        return result;
    }


    /*
     * ---------- Number ----------
     */
    if (parser->current_token.type == TOKEN_NUMBER)
    {
        char number[64];

        strcpy(number,
               parser->current_token.lexeme);


        SymbolType number_type;


        /*
         * If the literal contains '.',
         * treat it as a float.
         *
         * 10    -> int
         * 10.5  -> float
         */
        if (strchr(number, '.') != NULL)
        {
            number_type = SYMBOL_FLOAT;
        }
        else
        {
            number_type = SYMBOL_INT;
        }


        ExprResult result =
            make_result(number,
                        number_type);


        advance(parser);

        return result;
    }


    /*
     * ---------- Parenthesized expression ----------
     */
    if (parser->current_token.type == TOKEN_LPAREN)
    {
        advance(parser);


        ExprResult result =
            parse_expression(parser);


        if (parser->current_token.type == TOKEN_RPAREN)
        {
            advance(parser);
        }
        else
        {
            syntax_error(parser,
                         "Expected ')'");
        }


        return result;
    }


    /*
     * ---------- Invalid factor ----------
     */
    syntax_error(parser,
                 "Expected identifier, number, or '('");


    /*
     * Return a dummy value so parsing can continue.
     */
    advance(parser);


    ExprResult result =
        make_result("ERROR",
                    SYMBOL_INT);

    result.valid = 0;

    return result;
}


/* ---------- Status ---------- */

int parser_has_errors(const Parser *parser)
{
    return parser->error_count > 0;
}