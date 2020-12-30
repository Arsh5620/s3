#ifndef PARSER_INCLUDE_GAURD
#define PARSER_INCLUDE_GAURD

#include <stdio.h>
#include "lexer.h"
#include "../ssbs/list.h"

enum parser_wait_enum {
	PARSER_WAIT_KEY
	, PARSER_WAIT_ASSIGN
	, PARSER_WAIT_VALUE
	, PARSER_WAIT_NONE
	, PARSER_WAIT_END
};

enum parser_status_enum {
	PARSER_NOERROR	= 0
	, PARSER_EOF
	, PARSER_WARNINGS 
	, PARSER_WARNING_TRIM_STRING 
	, PARSER_ERRORS	= 256
	, PARSER_ERROR_UNEXPECTED_NEWLINE
	, PARSER_ERROR_UNEXPECTED_COMMENT
	, PARSER_ERROR_UNEXPECTED_ASSIGN
	, PARSER_ERROR_UNEXPECTED_STRING
	, PARSER_ERROR_UNEXPECTED_ILLEGAL
	, PARSER_ERROR_UNEXPECTED_TOOLONG
};

#define PARSER_OUT_LINE_INFO	"\n%.*s%.*s\n%*.*s^ -- %s"
#define PARSER_EMPTY_STRING		""

typedef struct {
	char *key;
	char *value;

	int key_length;
	int value_length;

	char is_valid;
} key_value_pair_s;

my_list_s parser_parse(lexer_status_s *status_r, char *buffer, int length);
my_list_s parser_parse_file(FILE *file);
void parser_release_list(my_list_s list);
key_value_pair_s parser_parse_next(lexer_s *lexer, lexer_status_s *err);
void parser_push_copy(my_list_s *list, key_value_pair_s pair);
void parser_print_status(lexer_s lexer, lexer_status_s status);
void parser_print_warn(lexer_s lexer, lexer_status_s status);
void parser_print_err(lexer_s lexer, lexer_status_s status);
void parser_print_lineinfo(lexer_s lexer, 
	lexer_status_s status, char error, char *message);

#endif // PARSER_INCLUDE_GAURD