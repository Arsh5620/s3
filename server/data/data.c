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

void
data_free (data_result_s result)
{
    my_list_free (result.list);
    hash_table_free (result.hash);
}

size_t
data_string_compare (void *memory, char *str, size_t strlen)
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
    hash_table_s table = hash_table_init (10, FALSE);

    for (long i = 0; i < list.count; ++i)
    {
        key_value_pair_s pair = *(key_value_pair_s *) my_list_get (list, i);

        int index = binary_search (
          data, sizeof (data_keys_s), length, pair.key, pair.key_length, data_string_compare);

        if (index != BINARYSEARCH_NORESULT)
        {
            data_keys_s attr = data[index];

            hash_table_bucket_s bucket
              = hash_table_get (table, (hash_input_u){.number = attr.attrib_code}, 0);

            if (bucket.is_occupied && bucket.key.number == attr.attrib_code)
            {
                continue;
            }

            hash_table_bucket_s new = {0};
            new.key.number = attr.attrib_code;
            new.value.number = i;

            hash_table_add (&table, new);
        }
    }
    return (table);
}
