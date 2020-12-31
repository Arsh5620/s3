#include "lexer.h"

lexer_s
lexer_init (char *buffer, int buffer_size)
{
    lexer_s lexer = {0};
    lexer.buffer = buffer;
    lexer.index = 0;
    lexer.size = buffer_size;
    return (lexer);
}

// increments index until either new line or EOF is reached
// returns 1 if EOF has been reached, and 0 otherwise.
int
lexer_skip_line (lexer_s *lexer)
{
    char eof = TRUE;

    while (lexer->index < lexer->size)
    {
        char c = lexer->buffer[lexer->index];
        if (c == '\n' || c == '\r')
        {
            eof = FALSE;
            break;
        }
        lexer->index++;
    }

    return (eof);
}

void
lexer_reset (lexer_s *lexer, lexer_status_s *lstatus, long size)
{
    lexer->index = 0;
    lexer->size = size;
    lstatus->base_index = 0;
}

lexer_token_s
lexer_next_token (lexer_s *lexer)
{
    lexer_token_s token = {0};
    char is_escape = 0;
    char is_string = 0;

    ulong index = lexer->index;
    while (lexer->index < lexer->size && token.token_type == 0)
    {
        char c = lexer->buffer[lexer->index];

        if (is_escape == FALSE)
        {
            switch (c)
            {
            case '#':
            {
                token.token_type = TOKEN_COMMENT_ONELINE;
            }
            break;

            case ':':
            case '=':
            {
                token.token_type = TOKEN_ASSIGNMENT;
            }
            break;

            case '\t':
            case ' ':
            {
                token.token_type = TOKEN_SKIP_SPCTAB;
            }
            break;

            case '\"':
            {
                if (is_string)
                {
                    token.token_type = TOKEN_ILLEGAL;
                }
            }
            break;

            default:
            {
                while (lexer->index < lexer->size)
                {
                    c = lexer->buffer[lexer->index];
                    char z = (c | 32);
                    // basically if the character is not alphanumeric, its an error
                    if ((z >= 'a' && z <= 'z') || (c >= '0' && c <= '9'))
                    {
                        lexer->index++;
                    }
                    else
                    {
                        break;
                    }
                }

                if (lexer->index == index)
                {
                    token.token_type = TOKEN_ILLEGAL;
                }
                else
                {
                    token.token_type = TOKEN_VALUE_RAW;
                    token.token_info.buffer = lexer->buffer;
                    token.token_info.index = index;
                    token.token_info.size = lexer->index - index;
                    continue;
                }
            }
            break;
            }
        }
        else if (is_escape == TRUE)
        {
            switch (c)
            {
            case '\"':
            {
                token.token_type = TOKEN_VALUE_QUOTEDSTRING;

                token.token_info.buffer = lexer->buffer;
                token.token_info.index = index;

                // The reason we are adding 1 is because we want both
                // quotes to be added to the string.
                token.token_info.size = (lexer->index + 1) - index;
            }
            break;
            }
        }

        switch (c)
        {
        case '\r':
        {
            token.token_type = TOKEN_CARRIAGERETURN;
        }
        break;

        case '\n':
        {
            token.token_type = TOKEN_NEWLINE;
        }
        break;

        case '\"':
        {
            is_escape = (is_escape == TRUE ? FALSE : TRUE);
        }
        break;
        }
        lexer->index++;
    }
    return (token);
}