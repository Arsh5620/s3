#ifndef DATA_INCLUDE_GAURD
#define DATA_INCLUDE_GAURD

// refer to CONFIGFORMAT for further information

#include "../files/file.h"
#include "../parser/parser.h"
#include "../ssbs/list.h"
#include "../data-structures/hash_table.h"
#include "../general/binarysearch.h"

enum data_type
{
    DATA_TYPE_STRING_S,
    DATA_TYPE_INT,
    DATA_TYPE_LONG,
    DATA_TYPE_SHORT,
    DATA_TYPE_BOOLEAN,
    DATA_TYPE_DOUBLE,
    DATA_TYPE_CHAR_PCOPY // use this if you want only string.
};

enum data_errors
{
    DATA_SUCCESS,
    DATA_CANNOT_CONVERT,
    DATA_KEY_NOT_FOUND
};

typedef struct
{
    char *string;
    ulong strlen;
    long attrib_code;
} data_keys_s;

typedef struct
{
    my_list_s list;
    hash_table_s hash;
} data_result_s;

#define DBP_KEY(str, code)                                                                         \
    (data_keys_s) { .string = str, .strlen = sizeof (str) - 1, .attrib_code = code }

void
data_free (data_result_s result);
size_t
data_string_compare (void *memory, char *str, size_t strlen);
hash_table_s
data_make_table (my_list_s list, data_keys_s *data, ulong length);
char *
data_get_string (my_list_s result_list, hash_table_s result_table, long key, int *error);
string_s
data_get_string_s (my_list_s result_list, hash_table_s result_table, long key, int *error);
key_value_pair_s
data_get_kvpair (my_list_s list, hash_table_s hash_table, long key, int *error);
#endif // CONFIG_INCLUDE_GAURD