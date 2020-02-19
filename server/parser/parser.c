#include "parser.h"
#include "../strings.h"
#include <string.h>
#include <stdio.h>

array_list_s parser_parse_start(char *buffer, int length)
{
    lexer_s lex = lexer_init(buffer, length);
    lexer_status_s status = {0};
    array_list_s list   = my_list_new(12, sizeof(key_value_pair_s));
    do
    {
        status.warnno   = 0;
        status.errno    = 0;
        status.status   = 0;
        key_value_pair_s pair = parser_parse_next(&lex, &status);
        my_list_push(&list, (char*)&pair);

        printf("key: {%.*s}, value:{%.*s}, status: %ld\n"
            , pair.key_length, pair.key
            , pair.value_length, pair.value
            , status.status);

        parser_print_status(lex, status);
        printf("****\n");
    } while (status.status != PARSER_STATUS_EOF 
                && status.errno == 0);
}

static char *errors[] = {
    "unexpected new line when expected string or operator.\n"
    , "unexpected comment when expected string or operator.\n"
    , "unexpected operator when expected string.\n"
    , "unexpected string when expected operator.\n"
    , "illegal string or operator at this location.\n"
};

static char *warnings[] = {
    "some data was found after entire statement was read,"
    " consider adding quotations.\n"
};

void parser_print_lineinfo(lexer_s lexer, lexer_status_s status, char is_err)
{
    printf("status code: %ld, line no: %ld, index no: %ld\n"
        , is_err?status.errno:status.warnno
        , status.lineno , is_err?status.e_charno:status.w_charno);

    int linelen = strings_count_until(
        lexer.buffer + status.base_index,
        lexer.size - status.base_index
        , '\n');

    printf("line length: %d\n", linelen);
    int stat    = is_err?status.e_charno:status.w_charno;
    int rest    = linelen - stat;

    char *st    = is_err ? "\x1B[1;91m\x1B[107m": "\x1B[34m" ;

    printf("%.*s%s%.*s\x1B[0m\n"
        , stat, lexer.buffer + status.base_index
        , st
        , rest, lexer.buffer + status.base_index + stat);
    printf("%*.*s^\n", stat, 0);
}

void parser_print_err(lexer_s lexer, lexer_status_s status) 
{
    parser_print_lineinfo(lexer, status, 1);
    long int errnum = status.errno - PARSER_ERRORS - 1;
    if(errnum >= 0 && errnum < sizeof(errors)/sizeof(char*))
        printf(errors[errnum]);
    else
        printf("unknown error occured, no information available.\n");
}

void parser_print_warn(lexer_s lexer, lexer_status_s status)
{
    parser_print_lineinfo(lexer, status, 0);
    long int warnno = status.warnno - PARSER_WARNINGS - 1;
    if(warnno >=0 && warnno < sizeof(warnings)/sizeof(char*))
        printf(warnings[warnno]);
    else 
        printf("unknown warning occured, possible bug, please report.\n");
}

void parser_print_status(lexer_s lexer, lexer_status_s status)
{
    if (status.warnno)
        parser_print_warn(lexer, status);
        
     if(status.errno)
        parser_print_err(lexer, status);
    
    if(status.errno == 0 && status.warnno ==0)
        printf("Warnings: 0, Errors: 0, Parsing completed successfully.\n");
}

key_value_pair_s parser_parse_next(lexer_s *lexer, lexer_status_s *lstatus)
{
    lexer_token_s token = {0};
    key_value_pair_s pair = {0};
    pair.is_valid   = 1;
    int status  = {0};
    
    lstatus->lineno++;
    lstatus->base_index = lexer->index;

    do {
        //only report the first error or warning.
        if(lstatus->warnno == 0)
            lstatus->w_charno   = lexer->index - lstatus->base_index;
        if(lstatus->errno == 0)
            lstatus->e_charno   = lexer->index - lstatus->base_index;

        token = lexer_next_token(lexer);
        switch (token.token_type)
        {
        case TOKEN_ILLEGAL:
            pair.is_valid   = 0;
            lstatus->errno  = PARSER_STATUS_ERRUNEXPECTED_ILLEGAL;
            break;
        case TOKEN_NEWLINE:
        case TOKEN_CARRIAGERETURN:
            if(!(status == PARSER_STATUS_WAIT_KEY 
                || status == PARSER_STATUS_WAIT_NONE)) {
                pair.is_valid   = 0;
                lstatus->errno = PARSER_STATUS_ERRUNEXPECTED_NEWLINE;
            }
            if(status == PARSER_STATUS_WAIT_NONE) {
                status  = PARSER_STATUS_WAIT_END;
            }
            break;
        case TOKEN_SKIP_SPCTAB:
            // if there are unneccssary spaces, or tabs, skip them.
            break;
        
        case TOKEN_COMMENT_ONELINE:
            if(!(status == PARSER_STATUS_WAIT_KEY
                || status == PARSER_STATUS_WAIT_NONE)) {
                pair.is_valid   = 0;
                lstatus->errno = PARSER_STATUS_ERRUNEXPECTED_COMMENT;
            }
            else 
                lexer_skip_line(lexer);
            break;
        
        case TOKEN_ASSIGNMENT:
            if (status != PARSER_STATUS_WAIT_ASSIGN)
            {
                pair.is_valid   = 0;
                lstatus->errno  = PARSER_STATUS_ERRUNEXPECTED_ASSIGN;
            }
            else 
                status  = PARSER_STATUS_WAIT_VALUE;
            break;

        case TOKEN_VALUE_QUOTEDSTRING:
            // everything is same as a raw string, just need to remove
            // the quotes, so index + 1, and size - 2
            token.token_info.index++;
            token.token_info.size -= 2;
            
        case TOKEN_VALUE_RAW:
            if(status == PARSER_STATUS_WAIT_KEY) {
                pair.key    = 
                    (token.token_info.buffer + token.token_info.index);
                pair.key_length = token.token_info.size;
                status  = PARSER_STATUS_WAIT_ASSIGN;
            }
            else if (status == PARSER_STATUS_WAIT_ASSIGN) {
                // illegal cannot have string when expected operator
                pair.is_valid   = 0;
                lstatus->errno  = PARSER_STATUS_ERRUNEXPECTED_STRING;
            } else if (status == PARSER_STATUS_WAIT_VALUE) {
                pair.value  = 
                    (token.token_info.buffer + token.token_info.index);
                pair.value_length   = token.token_info.size;
                status  = PARSER_STATUS_WAIT_NONE;
            } else if (status == PARSER_STATUS_WAIT_NONE) {
                lstatus->warnno  = PARSER_STATUS_WARNTRIM_STRING;
            }
            break;

        case TOKEN_EOF:
            pair.is_valid   = 0;
            lstatus->status  = PARSER_STATUS_EOF;
            break;
        default:
            printf("this event is not yet registered.\n");
            break;
        }
    } while(token.token_type != TOKEN_EOF 
            && pair.is_valid == 1
            && status != PARSER_STATUS_WAIT_END);
    
    return(pair);
}