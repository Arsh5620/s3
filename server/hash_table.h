# ifndef HASH_TABLE_INCLUDE_GAURD
# define HASH_TABLE_INCLUDE_GAURD
// This is the hash table that will be used for this program

typedef struct hash_table_entry_struct {
    unsigned long key;
    unsigned long value;
    char _bucket_occupied;
} hash_table_entry_s;

typedef struct hash_table_struct {
    hash_table_entry_s *hash_memory;

    int index;
    int length;
    int raw_size;

    double fill_factor;
} hash_table_s;

#define HASH_TABLE_DEFAULT_SIZE 128
#define HASH_TABLE_FILL_FACTOR  .75
#define HASH_TABLE_EXPAND_SIZE  2

unsigned int hash_table_hash(unsigned long in, unsigned int modulus);
hash_table_s hash_table_init();
int hash_table_add(hash_table_s *table, hash_table_entry_s entry);
hash_table_s hash_table_expand(hash_table_s *table);
hash_table_entry_s hash_table_get(hash_table_s table, unsigned long key);
# endif