#ifndef PARSER_INCLUDE_GAURD
#define PARSER_INCLUDE_GAURD

#include <stdio.h>
#include "lexer.h"
#include "../list.h" // please note that any allocations are not tracked

#define PARSER_STATUS_WAIT_KEY  0
#define PARSER_STATUS_WAIT_ASSIGN   1
#define PARSER_STATUS_WAIT_VALUE    2
#define PARSER_STATUS_WAIT_NONE 3
#define PARSER_STATUS_WAIT_END  4

#define PARSER_STATUS_NOERROR   0
#define PARSER_STATUS_EOF   1
#define PARSER_WARNINGS 2
#define PARSER_STATUS_WARNTRIM_STRING 3
#define PARSER_ERRORS 256
#define PARSER_STATUS_ERRUNEXPECTED_NEWLINE    257
#define PARSER_STATUS_ERRUNEXPECTED_COMMENT    258
#define PARSER_STATUS_ERRUNEXPECTED_ASSIGN 259
#define PARSER_STATUS_ERRUNEXPECTED_STRING 260
#define PARSER_STATUS_ERRUNEXPECTED_ILLEGAL 261
#define PARSER_STATUS_ERRUNEXPECTED_TOOLONG 262

typedef struct {
    char *key;
    char *value;

    int key_length;
    int value_length;

    char is_valid;
} key_value_pair_s;

typedef struct {
    char *buffer;
    array_list_s list;
    int error;
} parser_file_s;

array_list_s parser_parse(char *buffer, int length);
parser_file_s parser_parse_file(FILE *file);
key_value_pair_s parser_parse_next(lexer_s *lexer, lexer_status_s *err);
void parser_push_copy(array_list_s *list, key_value_pair_s pair);
void parser_print_status(lexer_s lexer, lexer_status_s status);
void parser_print_warn(lexer_s lexer, lexer_status_s status);
void parser_print_err(lexer_s lexer, lexer_status_s status);

#endif // PARSER_INCLUDE_GAURD