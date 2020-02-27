#include <string.h>
#include <stdio.h>
#include "parser.h"
#include "../strings.h"
#include "../file.h"
#include "../memory.h"

array_list_s parser_parse(char *buffer, int length)
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
        if(pair.key != NULL && pair.value != NULL)
            my_list_push(&list, (char*)&pair);
        parser_print_status(lex, status);
    } while (status.status != PARSER_STATUS_EOF 
                && status.errno == 0);

    return(list);
}

void parser_release_list(array_list_s list)
{
    for(long i = 0; i < list.index; ++i) {
        key_value_pair_s pair = *(key_value_pair_s*)my_list_get(list, i);
        if(pair.key)
            free(pair.key);
    }
}

void parser_push_copy(array_list_s *list, key_value_pair_s pair)
{
    if(pair.key_length == 0) // the node is a comment line
        return;

    key_value_pair_s node = {0};
    node.key    = 
        m_malloc(pair.key_length + pair.value_length
            , MEMORY_FILE_LINE);
    
    node.key_length = pair.key_length;
    node.value  = node.key + node.key_length;
    node.value_length   = pair.value_length;

    memmove(node.key, pair.key, pair.key_length);
    memmove(node.key + pair.key_length
        , pair.value, pair.value_length);
        
    my_list_push(list, (char*)&node);
    // printf("push key:%.*s, value:%.*s\n", 
    //     node.key_length, node.key, node.value_length, node.value);
}

/*
* please note that unlike parser_parse which will not do any malloc
* and copy the actual data but will contain an array_list_s that only has 
* references and sizes of the key:value pairs in memory buffer, 
* parser_parse_file will do memory allocation to store key:value pair
* this memory should be released using parser_release_list
* when no longer required. 
*/
array_list_s parser_parse_file(FILE *file)
{
    file_reader_s reader    = file_init_reader(file);

    // fill the file initially. 
    file_reader_fill(&reader, 0, reader.maxlength);

    lexer_s lex = lexer_init(reader.buffer, reader.readlength);
    lexer_status_s status = {0};
    array_list_s list   = my_list_new(12, sizeof(key_value_pair_s));
    key_value_pair_s pair = {0};

    do
    {
        status.warnno   = 0;
        status.errno    = 0;
        status.status   = 0;
        pair = parser_parse_next(&lex, &status);
        
        if(pair.key != NULL && pair.value != NULL) {
            parser_push_copy(&list, pair);
        }
        parser_print_status(lex, status);

        if(status.status == PARSER_STATUS_EOF) {
            // Now we should attempt to do more read and reset lexer
            if(status.base_index == 0 
                && reader.readlength == reader.maxlength) {
                printf("statment size exceeds the buffer size.\n");
                status.errno  = PARSER_STATUS_ERRUNEXPECTED_TOOLONG;
                parser_print_lineinfo(lex, status, 1);
                return(list);
            }

            long bytes_move = reader.readlength - status.base_index;
            long bytes_fill = reader.maxlength - bytes_move;

            if(bytes_move > 0)
                memmove(reader.buffer
                    , reader.buffer + status.base_index, bytes_move);

            file_reader_fill(&reader, bytes_move, bytes_fill);

            reader.readlength += bytes_move;
            lexer_reset(&lex, &status, reader.readlength);
        }
    } while ((reader.is_eof  == 0 || reader.readlength > 0)
                && status.errno == 0);
    
    if(status.status == PARSER_STATUS_EOF && pair.is_valid)
        parser_push_copy(&list, pair);
        
    return(list);
}

static char *errors[] = {
    "unexpected new line when expected string or operator.\n"
    , "unexpected comment when expected string or operator.\n"
    , "unexpected operator when expected string.\n"
    , "unexpected string when expected operator.\n"
    , "illegal character or operator at this location.\n"
};

static char *warnings[] = {
    "some data was found after entire statement was read,"
    " consider adding quotations.\n"
};

void parser_print_lineinfo(lexer_s lexer, 
    lexer_status_s status, char is_err)
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

    char *c = 0;
    printf("%*.*s^\n", stat, 0, c);
}

void parser_print_err(lexer_s lexer, lexer_status_s status) 
{
    parser_print_lineinfo(lexer, status, 1);
    long int errnum = status.errno - PARSER_ERRORS - 1;
    if(errnum >= 0 && errnum < sizeof(errors)/sizeof(char*))
        printf("\x1B[0;31m%s\x1B[0m", errors[errnum]);
    else
        printf("unknown error occured, no information available.\n");
}

void parser_print_warn(lexer_s lexer, lexer_status_s status)
{
    parser_print_lineinfo(lexer, status, 0);
    long int warnno = status.warnno - PARSER_WARNINGS - 1;
    if(warnno >=0 && warnno < sizeof(warnings)/sizeof(char*))
        printf("\x1B[0;32m%s\x1B[0m", warnings[warnno]);
    else 
        printf("unknown warning occured, possible bug, please report.\n");
}

void parser_print_status(lexer_s lexer, lexer_status_s status)
{
    if (status.warnno)
        parser_print_warn(lexer, status);

     if(status.errno)
        parser_print_err(lexer, status);
}

key_value_pair_s parser_parse_next(lexer_s *lexer, lexer_status_s *lstatus)
{
    lexer_token_s token = {0};
    key_value_pair_s pair = {0};
    pair.is_valid   = 1;
    int status  = {0};
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
            lstatus->lineno++;
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
            else {
                if(lexer_skip_line(lexer)){
                    lstatus->status  = PARSER_STATUS_EOF;
                    pair.is_valid   = 0;
                }
                   
                if(status == PARSER_STATUS_WAIT_KEY )  {
                    pair.is_valid   = 0;
                }
            }
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
            if(status != PARSER_STATUS_WAIT_NONE
                && status != PARSER_STATUS_WAIT_END)
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