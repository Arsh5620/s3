#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "config.h"
#include "../memdbg/memory.h"

size_t config_search_compare(void *memory, char *str, size_t strlen)
{
	struct config_parse *address    = (struct config_parse*) memory;
	 // order does matter
	int cmp = memcmp(str, address->key.address, address->key.length);

	if(cmp == 0 && address->key.length > strlen)
		cmp--;
	else if(cmp == 0 && address->key.length < strlen)
		cmp++;
	
	return(cmp);
}

void config_read_all(my_list_s list, struct config_parse *configs
	, int config_count, char *struct_memory)
{
	for (int i=0; i<list.count; ++i) {
		key_value_pair_s *pair = (key_value_pair_s*) my_list_get(list, i);
		int code  = binary_search(configs
			, sizeof(struct config_parse)
			, config_count
			, pair->key, pair->key_length
			, config_search_compare);
		if (code >= 0) {
			struct config_parse config	= configs[code];
			config_copy_data(struct_memory
				, config.offset, config.size, pair, config.type);
		}
	}
}

void config_copy_data(char *struct_memory, int offset
	, int size, key_value_pair_s *pair, enum config_data_type type)
{
	switch (type)
	{
	case CONFIG_TYPE_STRING:
		{
			char *memory	= 
				m_calloc(pair->value_length + 1, MEMORY_FILE_LINE);

			/*
			 *	first we are copying the actual pointer to the struct and 
			 * 	then we are copying the strings.
			 */
			memcpy((struct_memory + offset), &memory, sizeof(char*));
			memcpy(memory, pair->value, pair->value_length);
		}
		break;
	case CONFIG_TYPE_INT:
		{
			*(int*)(struct_memory + offset) = 
				(int) strtol(pair->value, NULL, 10);
		}
		break;
	case CONFIG_TYPE_LONG:
		{
			*(long*)(struct_memory + offset) = 
				(long) strtol(pair->value, NULL, 10);
		}
		break;
	case CONFIG_TYPE_SHORT:
		{
			*(short*)(struct_memory + offset) = (
					short) strtol(pair->value, NULL, 10);
		}
		break;
	case CONFIG_TYPE_ENUM:
		{
			puts("Not yet implemented");
			exit(1);
		}
		break;
	case CONFIG_TYPE_DOUBLE:
		{
			*(struct_memory + offset) = strtod(pair->value, NULL);
		}
		break;
	
	default:
		assert(type);
		break;
	}
}

/* 
 *	To use the config_parse_files you will need to first create your 
 *	bindings for the struct config_parse, and then you will pass this into 
 *	the functions as configs. See example usage in the Config.md
*/
void config_parse_files(char *filename, struct config_parse *configs
	, int config_count, char *struct_memory) 
{
	FILE *config	= fopen(filename, FILE_MODE_READONLY);
	my_list_s parsed	= parser_parse_file(config);
	config_read_all(parsed, configs, config_count, struct_memory);
	fclose(config);
	parser_release_list(parsed);
}