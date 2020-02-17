#include "lexer.h"

lexer_s lexer_init(char *buffer, int buffer_size)
{
    lexer_s lexer;
    lexer.buffer    = buffer;
    lexer.index = 0;
    lexer.size  = buffer_size;
    return(lexer);
}

lexer_token_s lexer_next_token(lexer_s *lexer)
{
    lexer_token_s token = {0};
    char is_escape  = 0;
    char is_string  = 0;

    while (lexer->index < lexer->size && token.token_type == 0) {
        char c  = lexer->buffer[lexer->index];
        switch (c)
        {
        case '#':
            if (is_escape == 0) {
                token.token_type    = TOKEN_COMMENT_ONELINE;
                continue;
            }
            break;
        case '=':
            if (is_escape == 0) {
                token.token_type    = TOKEN_ASSIGNMENT;
                continue;
            }
            break;
        case '\t':
        case ' ': 
            if (is_escape == 0) {
                token.token_type    = TOKEN_SKIP_SPCTAB;
                continue;
            }
            break;
        case '\r':
            token.token_type    = TOKEN_CARRIAGERETURN;
            continue;
        case '\n':
            token.token_type    = TOKEN_NEWLINE;
            continue;
        case '\"':
            if(is_string && is_escape == 0) {
                token.token_type    = TOKEN_ILLEGAL;
                continue;
            }
            if(is_escape == 1) {
                token.token_type    = TOKEN_VALUE_QUOTEDSTRING;
                continue;
            }
            is_escape   = !is_escape;
            break;
        default:
            while (lexer->index < lexer->size) {
                c  = lexer->buffer[lexer->index];
                if(((c >= 'A' && c <= 'Z')
                    || (c >='a' && c <= 'z')
                    || (c >= '0' && c <= '9')))
                    lexer->index++;
                else break;
            }
            token.token_type    = TOKEN_VALUE_RAW;
            continue;
        }
        lexer->index++;
    }
    return(token);
}