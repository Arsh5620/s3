#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "data.h"

char *
data_get_string (my_list_s result_list, hash_table_s result_table, long key, int *error)
{
    return data_get_kvpair (result_list, result_table, key, error).value;
}

string_s
data_get_string_s (my_list_s result_list, hash_table_s result_table, long key, int *error)
{
    key_value_pair_s pair = data_get_kvpair (result_list, result_table, key, error);
    return (string_s){.address = pair.value, .length = pair.value_length};
}

key_value_pair_s
data_get_kvpair (my_list_s list, hash_table_s hash_table, long key, int *error)
{
    hash_input_u key_u = {.number = key};
    hash_table_bucket_s bucket = hash_table_get (hash_table, key_u, 0);
    key_value_pair_s *pair;

    if (bucket.is_occupied == 1)
    {
        pair = ((key_value_pair_s *) my_list_get (list, bucket.value.number));
    }
    else
    {
        if (error != NULL)
        {
            *error = (DATA_KEY_NOT_FOUND);
        }
        
        return (key_value_pair_s){0};
    }

    if (error != NULL)
    {
        *error = SUCCESS;
    }
    return *pair;
}

data_result_s
data_parse_files (char *filename, data_keys_s *keys, int key_count)
{
    FILE *config = fopen (filename, FILE_MODE_READBINARY);
    my_list_s parsed = parser_parse_file (config);
    fclose (config);
    hash_table_s table = data_make_table (parsed, keys, key_count);
    data_result_s result;
    result.list = parsed;
    result.hash = table;
    return (result);
}

void
data_free (data_result_s result)
{
    my_list_free (result.list);
    hash_table_free (result.hash);
}

size_t
data_key_compare (void *memory, char *str, size_t strlen)
{
    data_keys_s *s = (data_keys_s *) memory;

    int cmp = memcmp (str, s->string, strlen); // order does matter it is a - b

    if (cmp == 0 && s->strlen > strlen)
    {
        cmp--;
    }
    else if (cmp == 0 && s->strlen < strlen)
    {
        cmp++;
    }
    return (cmp);
}

/*
 * this function will go through the list of the key:value pairs
 * and add those key value pairs to a hash table, while making
 * sure that there are not duplicates, if duplicates are found
 * the key:value pair found later is ignored.
 */
hash_table_s
data_make_table (my_list_s list, data_keys_s *data, ulong length)
{
    int count = list.count;
    hash_table_s table = hash_table_init (10, 0);

    for (long i = 0; i < count; ++i)
    {
        key_value_pair_s pair = *(key_value_pair_s *) my_list_get (list, i);

        int index = binary_search (
          data, sizeof (data_keys_s), length, pair.key, pair.key_length, data_key_compare);

        // will be -1 if an attribute is not supported (YET!)
        // which is also ignored.
        if (index != -1)
        {
            data_keys_s attr = data[index];

            hash_table_bucket_s b
              = hash_table_get (table, (hash_input_u){.number = attr.attrib_code}, 0);

            if (b.is_occupied)
            {
                continue;
            }

            hash_table_bucket_s bucket = {0};
            bucket.key.number = attr.attrib_code;
            bucket.value.number = i;

            hash_table_add (&table, bucket);
        }
    }
    return (table);
}
