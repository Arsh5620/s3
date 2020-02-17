#ifndef LEXER_INCLUDE_GAURD
#define LEXER_INCLUDE_GAURD

typedef struct {
    char *buffer;
    long int index;
    long int size;
} lexer_s;

enum lexer_tokens {
    TOKEN_EOF   = 128
    , TOKEN_KEY_NAME
    , TOKEN_ASSIGNMENT
    , TOKEN_VALUE_RAW
    , TOKEN_VALUE_STRING
    , TOKEN_VALUE_LONG
    , TOKEN_VALUE_DOUBLE
    , TOKEN_COMMENT_START
    , TOKEN_COMMENT_END
    , TOKEN_COMMENT_ONELINE
    , TOKEN_SKIP_SPCTAB
    , TOKEN_NEWLINE
    , TOKEN_CARRIAGERETURN 
    // carriage return is used on windows along with new line
    , TOKEN_ILLEGAL
    , TOKEN_VALUE_QUOTEDSTRING
};

typedef struct {
    enum lexer_tokens token_type;
} lexer_token_s;

lexer_s lexer_init(char *buffer, int buffer_size);
lexer_token_s lexer_next_token(lexer_s *lexer);

#endif // LEXER_INCLUDE_GAURD