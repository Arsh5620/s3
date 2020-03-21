# ifndef HASH_TABLE_INCLUDE_GAURD
# define HASH_TABLE_INCLUDE_GAURD

#include <stdlib.h>

typedef struct {
    char *key;
    long key_len;
    char *value;
    long value_len;
    char is_occupied;
} hash_table_bucket_s;

typedef struct hash_table_struct {
    hash_table_bucket_s *memory;
    size_t index;
    size_t size;
    size_t raw_size;
    size_t fill_factor;
    char is_string;
} hash_table_s;

#define HASH_TABLE_DEFAULT_SIZE 128
#define HASH_TABLE_FILL_FACTOR  .75
#define HASH_TABLE_EXPAND_SIZE  2
#define HASH_OCCUPIED   1

// hash table init long key
hash_table_s hash_table_initl();

// hash table init string key
hash_table_s hash_table_inits();

// hash table init custom with n elements buffer
hash_table_s hash_table_initn(long size, char is_key_string);

hash_table_s hash_table_expand(hash_table_s *table);
void hash_table_add(hash_table_s *table, hash_table_bucket_s entry);
hash_table_bucket_s hash_table_get(hash_table_s table 
    , char* key, size_t key_length);

# endif