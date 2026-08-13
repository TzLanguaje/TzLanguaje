#ifndef TZLANG_LEXER_H
#define TZLANG_LEXER_H

typedef enum {
    TOKEN_EOF,

    TOKEN_IDENTIFIER,
    TOKEN_NUMBER,
    TOKEN_STRING,

   TOKEN_VARIABLE,
    TOKEN_IMPRIMIR,

    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_STAR,
    TOKEN_SLASH,

    TOKEN_EQUAL,
    TOKEN_SEMICOLON,

    TOKEN_LPAREN,
    TOKEN_RPAREN
} TokenType;

typedef struct {
    TokenType type;
    char *value;
    int line;
} Token;



Token *lexer_tokenize(const char *source, int *token_count);
void lexer_free_tokens(Token *tokens, int token_count);
const char *token_type_name(TokenType type);

#endif

