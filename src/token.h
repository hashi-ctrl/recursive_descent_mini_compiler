#ifndef TOKEN_H
#define TOKEN_H

typedef enum {
    TOKEN_INT,
    TOKEN_FLOAT,

    TOKEN_IDENTIFIER,
    TOKEN_NUMBER,

    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_MULTIPLY,
    TOKEN_DIVIDE,

    TOKEN_ASSIGN,

    TOKEN_LPAREN,
    TOKEN_RPAREN,

    TOKEN_SEMICOLON,

    TOKEN_EOF,
    TOKEN_UNKNOWN

} TokenType;

typedef struct {
    TokenType type;
    char lexeme[100];
} Token;

#endif
