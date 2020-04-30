#ifndef CONFIG_INCLUDE_GAURD
#define CONFIG_INCLUDE_GAURD

// refer to CONFIGFORMAT for further information

#include "../files/file.h"
#include "../parser/parser.h"
#include "../data-structures/list.h"
#include "../data-structures/hash_table.h"
#include "../general/binarysearch.h"

enum data_type {
	CONFIG_TYPE_STRING_S
	, CONFIG_TYPE_INT
	, CONFIG_TYPE_LONG
	, CONFIG_TYPE_SHORT
	, CONFIG_TYPE_BOOLEAN
	, CONFIG_TYPE_DOUBLE
	, CONFIG_TYPE_CHAR_PCOPY // use this if you want only string. 
};

enum data_errors
{
	DATA_SUCCESS	
	, DATA_CANNOT_CONVERT
	, DATA_KEY_NOT_FOUND
};

typedef struct {
	char *string;
	ulong strlen;
	long attrib_code;
} data_keys_s;

typedef struct {
	my_list_s list;
	hash_table_s hash;
} data_result_s;

#define DBP_KEY(str, code) \
	(data_keys_s) { \
		.string = str \
		, .strlen = sizeof(str) - 1 \
		, .attrib_code = code \
	}

int data_convert(key_value_pair_s *pair, enum data_type type
	, char *out, ulong length);
key_value_pair_s *data_get_key_value(data_result_s result, long key);
data_result_s data_parse_files(char *filename
	, data_keys_s *keys, int key_count);
hash_table_s data_make_table(my_list_s list
	, data_keys_s *data, ulong length);
size_t data_key_compare(void *memory, char *str, size_t strlen);
void data_free(data_result_s result);
int data_get_and_convert(data_result_s result, long key
	, enum data_type type, char *memory, long length);
#endif // CONFIG_INCLUDE_GAURD