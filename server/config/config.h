#ifndef CONFIG_INCLUDE_GAURD
#define CONFIG_INCLUDE_GAURD

// refer to CONFIGFORMAT for further information

#include "../files/file.h"
#include "../data-structures/list.h"
#include "../parser/parser.h"
#include "../general/strings.h"
#include "../general/binarysearch.h"

enum config_data_type {
	CONFIG_TYPE_STRING
	, CONFIG_TYPE_STRING_S
	, CONFIG_TYPE_INT
	, CONFIG_TYPE_LONG
	, CONFIG_TYPE_SHORT
	, CONFIG_TYPE_BOOLEAN
	, CONFIG_TYPE_DOUBLE
	, CONFIG_TYPE_ENUM
};

struct config_parse {
	string_s key;	/* key include string such as "action" */
	int	code;	/* code for the key, must be unique */
	int offset;	/* offset to the memory in the struct for this key */
	int size;	/* size of the variable in the struct */
	enum config_data_type type;
};

#define KEY_STRING(x) (string_s){.address=x, .length=sizeof(x) - 1}
#define STRUCT_MEMBER_SIZE(STRUCT_NAME, MEMBER_NAME) \
	sizeof(((STRUCT_NAME*)0)->MEMBER_NAME)
#define STRUCT_CONFIG_PARSE(_key, _code, _struct, _member, _type) \
	{ \
		.key = KEY_STRING(_key) \
		, .code = _code \
		, .offset = offsetof(_struct, _member) \
		, .size	= STRUCT_MEMBER_SIZE(_struct, _member) \
		, .type = _type \
	}

void config_copy_data(char *struct_memory, int offset
	, int size, key_value_pair_s *pair, enum config_data_type type);
void config_read_all(my_list_s list, struct config_parse *configs
	, int config_count, char *struct_memory);
void config_parse_files(char *filename, struct config_parse *configs
	, int config_count, char *struct_memory);
void config_free_all(struct config_parse *configs
	, int config_count, char *structs);
#endif // CONFIG_INCLUDE_GAURD