#ifndef PARSER_INCLUDE_GAURD
#define PARSER_INCLUDE_GAURD

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

typedef struct {
    char *key;
    char *value;

    int key_length;
    int value_length;

    char is_valid;
} key_value_pair_s;

array_list_s parser_parse(char *buffer, int length);
key_value_pair_s parser_parse_next(lexer_s *lexer, lexer_status_s *err);
void parser_print_status(lexer_s lexer, lexer_status_s status);
void parser_print_warn(lexer_s lexer, lexer_status_s status);
void parser_print_err(lexer_s lexer, lexer_status_s status);

#endif // PARSER_INCLUDE_GAURD