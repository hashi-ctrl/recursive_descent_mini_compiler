#include <stdio.h>
#include <string.h>

#include "parser.h"
#include "lexer.h"


/* ---------- Helper structures ---------- */

typedef struct {
    char value[64];
} ExprResult;


/* ---------- Forward declarations ---------- */

static void advance(Parser *parser);
static void syntax_error(Parser *parser, const char *message);
static void expect(Parser *parser, TokenType type);

static void parse_statement(Parser *parser);
static void parse_declaration(Parser *parser);
static void parse_assignment(Parser *parser);

static ExprResult parse_expression(Parser *parser);
static ExprResult parse_term(Parser *parser);
static ExprResult parse_factor(Parser *parser);

static ExprResult make_result(const char *value);
static ExprResult make_temp(Parser *parser);


/* ---------- Parser initialization ---------- */

void parser_init(Parser *parser, const char *source)
{
    parser->source = source;
    parser->position = 0;
    parser->error_count = 0;
    parser->temp_count = 0;

    parser->current_token =
        getNextToken(parser->source, &parser->position);
}


/* ---------- Token handling ---------- */

static void advance(Parser *parser)
{
    parser->current_token =
        getNextToken(parser->source, &parser->position);
}


static void syntax_error(Parser *parser, const char *message)
{
    printf("Syntax Error: %s", message);

    if (parser->current_token.lexeme[0] != '\0')
        printf(" (near '%s')", parser->current_token.lexeme);

    printf("\n");

    parser->error_count++;
}


static void expect(Parser *parser, TokenType type)
{
    if (parser->current_token.type == type)
    {
        advance(parser);
    }
    else
    {
        syntax_error(parser, "Unexpected token");
    }
}


/* ---------- Expression helpers ---------- */

static ExprResult make_result(const char *value)
{
    ExprResult result;

    strncpy(result.value, value, sizeof(result.value) - 1);
    result.value[sizeof(result.value) - 1] = '\0';

    return result;
}


static ExprResult make_temp(Parser *parser)
{
    ExprResult result;

    parser->temp_count++;

    snprintf(result.value,
             sizeof(result.value),
             "t%d",
             parser->temp_count);

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
        syntax_error(parser, "Expected declaration or assignment");

        /*
         * Skip the problematic token so parsing can continue.
         */
        advance(parser);
    }
}


/* ---------- Declaration ---------- */

static void parse_declaration(Parser *parser)
{
    /* Consume int / float */
    advance(parser);

    if (parser->current_token.type != TOKEN_IDENTIFIER)
    {
        syntax_error(parser, "Expected identifier after type");
        return;
    }

    /* Consume identifier */
    advance(parser);

    expect(parser, TOKEN_SEMICOLON);
}


/* ---------- Assignment ---------- */

static void parse_assignment(Parser *parser)
{
    char identifier[64];

    strcpy(identifier, parser->current_token.lexeme);

    /* Consume identifier */
    expect(parser, TOKEN_IDENTIFIER);

    /* Expect '=' */
    if (parser->current_token.type != TOKEN_ASSIGN)
    {
        syntax_error(parser, "Expected '=' after identifier");
        return;
    }

    advance(parser);

    /* Parse expression */
    ExprResult expression = parse_expression(parser);

    /* Expect semicolon */
    if (parser->current_token.type != TOKEN_SEMICOLON)
    {
        syntax_error(parser, "Expected ';' after expression");
        return;
    }

    advance(parser);

    /*
     * Final assignment in TAC.
     */
    printf("%s = %s\n", identifier, expression.value);
}


/* ---------- Expression ---------- */

/*
 * expression → term { (+ | -) term }
 */

static ExprResult parse_expression(Parser *parser)
{
    ExprResult left = parse_term(parser);

    while (parser->current_token.type == TOKEN_PLUS ||
           parser->current_token.type == TOKEN_MINUS)
    {
        TokenType operator = parser->current_token.type;

        advance(parser);

        ExprResult right = parse_term(parser);
        ExprResult temp = make_temp(parser);

        if (operator == TOKEN_PLUS)
        {
            printf("%s = %s + %s\n",
                   temp.value,
                   left.value,
                   right.value);
        }
        else
        {
            printf("%s = %s - %s\n",
                   temp.value,
                   left.value,
                   right.value);
        }

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
    ExprResult left = parse_factor(parser);

    while (parser->current_token.type == TOKEN_MULTIPLY ||
           parser->current_token.type == TOKEN_DIVIDE)
    {
        TokenType operator = parser->current_token.type;

        advance(parser);

        ExprResult right = parse_factor(parser);
        ExprResult temp = make_temp(parser);

        if (operator == TOKEN_MULTIPLY)
        {
            printf("%s = %s * %s\n",
                   temp.value,
                   left.value,
                   right.value);
        }
        else
        {
            printf("%s = %s / %s\n",
                   temp.value,
                   left.value,
                   right.value);
        }

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
    if (parser->current_token.type == TOKEN_IDENTIFIER ||
        parser->current_token.type == TOKEN_NUMBER)
    {
        ExprResult result =
            make_result(parser->current_token.lexeme);

        advance(parser);

        return result;
    }

    if (parser->current_token.type == TOKEN_LPAREN)
    {
        advance(parser);

        ExprResult result = parse_expression(parser);

        if (parser->current_token.type == TOKEN_RPAREN)
        {
            advance(parser);
        }
        else
        {
            syntax_error(parser, "Expected ')'");
        }

        return result;
    }

    syntax_error(parser, "Expected identifier, number, or '('");

    /*
     * Return a dummy value so parsing can continue.
     */
    advance(parser);

    return make_result("ERROR");
}


/* ---------- Status ---------- */

int parser_has_errors(const Parser *parser)
{
    return parser->error_count > 0;
}
