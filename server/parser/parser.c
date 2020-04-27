#include <string.h>
#include <stdio.h>
#include "parser.h"
#include "../general/string.h"
#include "../files/file.h"
#include "../memdbg/memory.h"
#include "../output/output.h"

my_list_s parser_parse(lexer_status_s *status_r, char *buffer, int length)
{
	lexer_s lexer	= lexer_init(buffer, length);
	my_list_s list	= my_list_new(12, sizeof(key_value_pair_s));

	lexer_status_s status	= {0};
	do
	{
		status.warnno   = 0;
		status.errno    = 0;
		status.status   = 0;
		key_value_pair_s pair = parser_parse_next(&lexer, &status);
		if (pair.key != NULL && pair.value != NULL)
		{
			my_list_push(&list, (char*)&pair);
		}
		parser_print_status(lexer, status);
	} 
	while (status.status != PARSER_EOF && status.errno == 0);
				
	*status_r	= status;
	return(list);
}

void parser_release_list(my_list_s list)
{
	for (long i = 0; i < list.count; ++i) 
	{
		key_value_pair_s pair = *(key_value_pair_s*)my_list_get(list, i);
		if (pair.key != NULL)
		{
			m_free(pair.key, MEMORY_FILE_LINE);
		}
	}
	my_list_free(list);
}

void parser_push_copy(my_list_s *list, key_value_pair_s pair)
{
	if (pair.key_length == 0) // the node can be a comment
	{
		return;
	}

	key_value_pair_s node = {0};
	ulong alloc	= pair.key_length + pair.value_length;

	// + 1 is for null terminator
	node.key    = m_malloc(alloc + 1, MEMORY_FILE_LINE);
	*(node.key + alloc)	= 0;
	
	node.value  = node.key + pair.key_length;
	node.key_length		= pair.key_length;
	node.value_length   = pair.value_length;

	memmove(node.key, pair.key, pair.key_length);
	memmove(node.key + pair.key_length, pair.value, pair.value_length);
	my_list_push(list, (char*)&node);
}

/*
* please note that unlike parser_parse which will not do any malloc
* and copy the actual data but will contain an array_list_s that only has 
* references and sizes of the key:value pairs in memory buffer, 
* parser_parse_file will do memory allocation to store key:value pair
* this memory should be released using parser_release_list
* when no longer required. 
*/
my_list_s parser_parse_file(FILE *file)
{
	// fill the file initially. 
	file_reader_s reader    = file_reader_init(file);
	file_reader_next(&reader, 0, reader.maxlength);

	lexer_s lexer	= lexer_init(reader.buffer, reader.readlength);
	my_list_s list	= my_list_new(12, sizeof(key_value_pair_s));

	lexer_status_s status	= {0};
	key_value_pair_s pair	= {0};

	do
	{
		// clear warnings and errors before calling parse next
		status.warnno   = 0;
		status.errno    = 0;
		status.status   = 0;
		pair = parser_parse_next(&lexer, &status);
		
		if (pair.key != NULL && pair.value != NULL) 
		{
			parser_push_copy(&list, pair);
		}

		parser_print_status(lexer, status);

		if (status.status == PARSER_EOF) {
			// Now we should attempt to do more read and reset lexer
			if (status.base_index == 0 
				&& reader.readlength == reader.maxlength) 
			{
				status.errno  = PARSER_ERROR_UNEXPECTED_TOOLONG;
				parser_print_lineinfo(lexer, status, 1, PARSER_EMPTY_STRING);
				return(list);
			}

			long bytes_move = reader.readlength - status.base_index;
			long bytes_fill = reader.maxlength - bytes_move;

			if (bytes_move > 0)
			{
				memmove(reader.buffer
					, reader.buffer + status.base_index, bytes_move);
			}

			file_reader_next(&reader, bytes_move, bytes_fill);

			reader.readlength += bytes_move;
			lexer_reset(&lexer, &status, reader.readlength);
		}
	} 
	while ((reader.eof  == 0 || reader.readlength > 0)
				&& status.errno == 0);
	
	if (status.status == PARSER_EOF && pair.is_valid)
	{
		parser_push_copy(&list, pair);
	}
	
	file_reader_close(&reader);
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
	lexer_status_s status, char error, char *message)
{
	char *base_pointer	= lexer.buffer + status.base_index;
	long max_n	= lexer.size - status.base_index;
	long length	= ((char*)memchr(base_pointer, '\n', max_n) - base_pointer);

	output_handle(OUTPUT_HANDLE_LOGS, LOGGER_LEVEL_WARN
		, PARSER_STDOUT_ERROR_STRING
		, error ? status.errno : status.warnno
		, status.lineno 
		, error ? status.e_charno : status.w_charno
		, length);

	int stat    = error ? status.e_charno : status.w_charno;
	int rest    = length - stat;

	char c = 0;

	output_handle(OUTPUT_HANDLE_LOGS, LOGGER_LEVEL_WARN
		, PARSER_OUT_LINE_INFO
		, stat, lexer.buffer + status.base_index
		, rest , lexer.buffer + status.base_index + stat
		, stat, 0, &c, message);
}

void parser_print_err(lexer_s lexer, lexer_status_s status) 
{
	long int errnum = status.errno - PARSER_ERRORS - 1;

	if (errnum >= 0 && errnum < sizeof(errors)/sizeof(char*))
	{
		parser_print_lineinfo(lexer, status, FALSE, errors[errnum]);
	}
}

void parser_print_warn(lexer_s lexer, lexer_status_s status)
{
	long warnno = status.warnno - PARSER_WARNINGS - 1;

	if (warnno >=0 && warnno < sizeof(warnings)/sizeof(char*))
	{
		parser_print_lineinfo(lexer, status, FALSE, warnings[warnno]);
	}
}

void parser_print_status(lexer_s lexer, lexer_status_s status)
{
	if (status.warnno)
	{
		parser_print_warn(lexer, status);
	}

	if (status.errno)
	{
		parser_print_err(lexer, status);
	}
}

key_value_pair_s parser_parse_next(lexer_s *lexer, lexer_status_s *lstatus)
{
	lexer_token_s token		= {0};
	key_value_pair_s pair	= {0};
	pair.is_valid   = TRUE;
	int status  	= {0};
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
		{
			pair.is_valid   = FALSE;
			lstatus->errno  = PARSER_ERROR_UNEXPECTED_ILLEGAL;
		}
		break;

		case TOKEN_NEWLINE:
		{
			lstatus->lineno++;
		}
		// after new line fall into carriage return
		case TOKEN_CARRIAGERETURN:
		{
			// If we are already in a statement that is not complete
			// flag an error
			if (status != PARSER_WAIT_KEY 
				&& status != PARSER_WAIT_NONE)
			{
				pair.is_valid	= FALSE;
				lstatus->errno	= PARSER_ERROR_UNEXPECTED_NEWLINE;
			}
			// else if entire statement has been read, flag the end
			else if (status == PARSER_WAIT_NONE)
			{
				status  = PARSER_WAIT_END;
			}
		}
		break;

		case TOKEN_SKIP_SPCTAB:
			// if there are unneccssary spaces, or tabs, ignore them.
			break;
		
		case TOKEN_COMMENT_ONELINE:
		{
			pair.is_valid	= FALSE;

			// if we have a one line comment, check to see 
			// if we are in the middle of a statement
			if(status != PARSER_WAIT_KEY
				&& status != PARSER_WAIT_NONE)
			{
				lstatus->errno	= PARSER_ERROR_UNEXPECTED_COMMENT;
			}
			else  if (lexer_skip_line(lexer) == TRUE)
			{
				lstatus->status	= PARSER_EOF;
			}
		}
		break;
		
		case TOKEN_ASSIGNMENT:
		{
			if (status != PARSER_WAIT_ASSIGN)
			{
				pair.is_valid   = FALSE;
				lstatus->errno  = PARSER_ERROR_UNEXPECTED_ASSIGN;
			}
			else 
			{
				status  = PARSER_WAIT_VALUE;
			}
		}
		break;

		case TOKEN_VALUE_QUOTEDSTRING:
		{
			// same as a raw token, just need to remove
			// the quotes, so index + 1, and size - 2
			token.token_info.index++;
			token.token_info.size -= 2;
		}
		// after setting up index and size fall into raw token	
		case TOKEN_VALUE_RAW:
		{
			char *key	= (token.token_info.buffer + token.token_info.index);
			if (status == PARSER_WAIT_KEY)
			{
				pair.key	= key;
				pair.key_length = token.token_info.size;
				status	= PARSER_WAIT_ASSIGN;
			}
			else if (status == PARSER_WAIT_ASSIGN)
			{
				// illegal cannot have string when expected operator
				pair.is_valid   = FALSE;
				lstatus->errno  = PARSER_ERROR_UNEXPECTED_STRING;
			} 
			else if (status == PARSER_WAIT_VALUE) 
			{
				pair.value	= key;
				pair.value_length   = token.token_info.size;
				status	= PARSER_WAIT_NONE;
			} 
			else if (status == PARSER_WAIT_NONE) 
			{
				lstatus->warnno  = PARSER_WARNING_TRIM_STRING;
			}
		}
		break;

		case TOKEN_NONE:
		{
			if (status != PARSER_WAIT_NONE
				&& status != PARSER_WAIT_END)
			{
				pair.is_valid	= FALSE;
			}
			lstatus->status	= PARSER_EOF;
		}
		break;
		
		default:
			printf("This event is not yet registered.\n");
			break;
		}
	} 
	while(token.token_type != TOKEN_NONE
			&& pair.is_valid == TRUE
			&& status != PARSER_WAIT_END);

	return(pair);
}