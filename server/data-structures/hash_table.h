# ifndef HASH_TABLE_INCLUDE_GAURD
# define HASH_TABLE_INCLUDE_GAURD

#include <stdlib.h>

typedef union {
	char *address;
	size_t number;
} hash_input_u;

typedef struct {
	/* union allows for us to use both addresses and numbers */
	hash_input_u key; 
	hash_input_u value;
    long key_len;
    long value_len;
    char is_occupied;
} hash_table_bucket_s;

typedef struct hash_table_struct {
    hash_table_bucket_s *memory;
    size_t count;
    size_t size;
    size_t fill;
    char is_string;
} hash_table_s;

#define HASH_EXPAND	2
#define HASH_EMPTY	0
#define HASH_FILL	.75
#define HASH_OCCUPIED   1
#define HASH_TOMBSTONE   1
#define HASH_DEFAULT	128

// hash table init long key
hash_table_s hash_table_initl();

// hash table init string key
hash_table_s hash_table_inits();

// hash table init custom with n elements buffer
hash_table_s hash_table_initn(long size, char is_key_string);
void hash_table_expand(hash_table_s *table);

void hash_table_add(hash_table_s *table, hash_table_bucket_s entry);
hash_table_bucket_s hash_table_get(hash_table_s table
    , hash_input_u key, size_t key_length);
void hash_table_free(hash_table_s table);

# endif