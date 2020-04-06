#ifndef PARSER_INCLUDE_GAURD
#define PARSER_INCLUDE_GAURD

#include <stdio.h>
#include "lexer.h"
#include "../data-structures/list.h"

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

my_list_s parser_parse(char *buffer, int length);
my_list_s parser_parse_file(FILE *file);
void parser_release_list(my_list_s list);
key_value_pair_s parser_parse_next(lexer_s *lexer, lexer_status_s *err);
void parser_push_copy(my_list_s *list, key_value_pair_s pair);
void parser_print_status(lexer_s lexer, lexer_status_s status);
void parser_print_warn(lexer_s lexer, lexer_status_s status);
void parser_print_err(lexer_s lexer, lexer_status_s status);
void parser_print_lineinfo(lexer_s lexer,
    lexer_status_s status, char is_err);

#endif // PARSER_INCLUDE_GAURD