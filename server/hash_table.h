# ifndef HASH_TABLE_INCLUDE_GAURD
# define HASH_TABLE_INCLUDE_GAURD
#include <stdlib.h>
/*
 * before using the hash table, here are some things to consider:
 * this hash table was designed for use by memory.c, even though 
 * it can be used by other parts of the program, doing so is not
 * recommended as the hash table is not able to use m_malloc like 
 * functions, and must use the operating system's malloc
 */

typedef struct {
    unsigned long key;
    unsigned long value;
    char is_occupied;
} hash_table_bucket_s;

typedef struct hash_table_struct {
    hash_table_bucket_s *memory;
    size_t index;
    size_t size;
    size_t raw_size;
    size_t fill_factor;
} hash_table_s;

#define HASH_TABLE_DEFAULT_SIZE 128
#define HASH_TABLE_FILL_FACTOR  .75
#define HASH_TABLE_EXPAND_SIZE  2

hash_table_s hash_table_init();
hash_table_s hash_table_expand(hash_table_s *table);
void hash_table_add(hash_table_s *table, hash_table_bucket_s entry);
unsigned long hash_table_hash(unsigned long in, unsigned long modulus);
hash_table_bucket_s hash_table_get(hash_table_s table, unsigned long key);

# endif