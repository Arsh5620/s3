#ifndef SRC_DATA_HASH_TABLE_
#define SRC_DATA_HASH_TABLE_

#include "general/common.h"
#include <stdlib.h>

#define HASH_OCCUPIED 1
#define HASH_TOMBSTONE 2

typedef struct {
    char *key;
    char *value;
    long key_length;
    long value_length;
    int occupied;
} hash_table_bucket_s;

typedef struct hash_table_struct {
    hash_table_bucket_s *memory;
    long count;
    long capacity;
    long capacity_max_fill;
    long collision_count;

    short expand_add;
    float expand_multiply;
    float hash_fill_multiply;
} hash_table_s;

hash_table_s
hash_table_alloc (long initial_count, float fill_percentage, float expand_multiply, short expand_add);
void
hash_table_free (hash_table_s table);

void
hash_table_add (hash_table_s *table, char *key, long key_length, char *value, long value_length);
void
hash_table_remove (hash_table_s *table, char *key, long key_length);

hash_table_bucket_s
hash_table_get (hash_table_s table, char *key, long key_length);

#endif // SRC_DATA_HASH_TABLE_