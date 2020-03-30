#include "hash_table.h"
#include <string.h>

/**
 * return value: unsigned int
 * this function calculates a hash given the in and the modulus
 * the modulus must be a power of 2
 */
unsigned long hash_long(unsigned long in, unsigned long modulus)
{
    unsigned int _a1    = in & 0xFFFFFFFF
                , _a2   = (~in >> 32) & 0xFFFFFFFF;
    unsigned long li = (((_a1 * 2726656) ^ (_a2 * 9931)) + _a2);
    li += ~li / modulus;
	return li & (modulus - 1);
}

unsigned long hash_string(char *memory, long len, unsigned long modulus)
{
    char *m1    = memory;
    unsigned long in    = 0;
    for(long i=0; i<len; ++i) {
        in += *m1;
        unsigned int _a1    = in & 0xFFFFFFFF
            , _a2   = (~in >> 32) & 0xFFFFFFFF;
        in = (((_a1 * 2726656) ^ (_a2 * 9931)) + _a2);
    }
    in += ~in / modulus;
    return in & (modulus - 1);
}

size_t hash_hash(char is_string , char *memory
    , size_t len, size_t modulus)
{
    size_t hash  = 0;
    if(is_string)
        hash    = hash_string(memory, len, modulus);
    else {
        size_t i = (size_t)memory;
        hash    = hash_long(i, modulus);
    }
    return(hash);
}

/*
 * hash_table_init inits a new hash_table_s structure 
 * it allocated memory for HASH_TABLE_DEFAULT_SIZE buckets, 
 * and returns the structure which can later be used to fill the 
 * table and make use of it. 
 */
hash_table_s hash_table_initl()
{
    return(hash_table_initn(HASH_TABLE_DEFAULT_SIZE, 0));
}

hash_table_s hash_table_inits()
{
    return(hash_table_initn(HASH_TABLE_DEFAULT_SIZE, 1));
}

hash_table_s hash_table_initn(long size, char is_string)
{
    hash_table_s hash_table = {0};
    hash_table.size	= size;
    hash_table.raw_size	= size * sizeof(hash_table_bucket_s);
    hash_table.fill_factor	= size * HASH_TABLE_FILL_FACTOR;
    hash_table.memory	= 
        (hash_table_bucket_s*) calloc(hash_table.raw_size, 1);
    hash_table.is_string    = is_string;
    return(hash_table);
}

/*
 * as first parameter this functions expects handle returned by 
 * hash_table_init(), and then it tries to add the "entry" to the 
 * hash table using open address hashing. 
 */
void hash_table_add(hash_table_s *table, hash_table_bucket_s entry)
{
    size_t hash = 
        hash_hash(table->is_string, entry.key, entry.key_len, table->size);
    
    hash_table_bucket_s *destination = &table->memory[hash];
    
    while (destination->is_occupied == HASH_OCCUPIED)
    {
        ++destination;
        if(destination >= (table->memory + table->size))
            destination = table->memory;
    }

    entry.is_occupied   = HASH_OCCUPIED;
    *destination = entry;
    table->index++;

    if(table->index >= table->fill_factor)
        hash_table_expand(table);
}

/*
 * hash_table_expand should not be called by the user, 
 * it will already be called automatically by hash_table_add 
 * if the threshold for the entries exceed a certain limit. 
 * this function will then allocate memory for more elements, 
 * and will move and reassign elements from previous memory 
 * location to the new location. 
 * this function will release the previously allocated memory
 * if needs to.
 */
hash_table_s hash_table_expand(hash_table_s *table)
{
    unsigned int original_size  = table->size;

    table->index	= 0;
    table->size	*= HASH_TABLE_EXPAND_SIZE;
    table->raw_size = table->size * sizeof(hash_table_bucket_s);
    table->fill_factor	= table->size * HASH_TABLE_FILL_FACTOR;

    hash_table_bucket_s *source  = table->memory, *source_copy = source;
    table->memory  = (hash_table_bucket_s*)calloc(table->raw_size, 1);

    while(original_size--) {
        if(source->is_occupied)
            hash_table_add(table, *source);
        source++;
    }

    if(source_copy != NULL)
        free(source_copy);

    return(*table);
}

/*
 * hash_table_get is used to get the key:value pair from hash table
 * returns: hash_table_bucket_s
 */
hash_table_bucket_s hash_table_get(hash_table_s table
    , char* key, size_t key_length)
{    
    size_t hash = hash_hash(table.is_string, key, key_length, table.size);
    hash_table_bucket_s *entry = (table.memory + hash), result = {0};

    while(((table.is_string == 0 && entry->key != key)
        || (table.is_string == 1 && entry->key != 0
		&& memcmp(entry->key, key, key_length)))
        && entry->is_occupied) {
        entry++;
        if(entry >= table.memory + table.size)
            entry = table.memory;
    }

    if(entry->is_occupied)
        result  = *entry;

    return result;
}

void hash_table_free(hash_table_s table) 
{
	if(table.memory)
		free(table.memory);
}