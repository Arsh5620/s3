#ifndef LEXER_INCLUDE_GAURD
#define LEXER_INCLUDE_GAURD
#include "../general/define.h"
// please read file configformat.2 for more information on how this lexer
// produces tokens. 

typedef struct{
	char *buffer;
	long int index;
	long int size;
} lexer_s, lexer_string_s;

typedef struct{
	long status; // status can only be noerror or eof
	long err_no; // if any error occurs, first one is recorded
	long warnno; // if any warn occurs, first one is recorded
	long lineno; // the line number we are working on right now.
	long base_index;
	long e_charno;
	long w_charno;
} lexer_status_s;

enum lexer_tokens {
	TOKEN_NONE	= 0
	// , TOKEN_KEY_NAME = 128
	, TOKEN_ASSIGNMENT = 128
	, TOKEN_VALUE_RAW
	// , TOKEN_VALUE_STRING
	// , TOKEN_VALUE_LONG
	// , TOKEN_VALUE_DOUBLE
	// , TOKEN_COMMENT_START
	// , TOKEN_COMMENT_END
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
	lexer_string_s  token_info;
} lexer_token_s;

lexer_s lexer_init(char *buffer, int buffer_size);
lexer_token_s lexer_next_token(lexer_s *lexer);
int lexer_skip_line(lexer_s *lexer);
void lexer_reset(lexer_s *lexer,lexer_status_s *lstatus, long size);

#endif // LEXER_INCLUDE_GAURD