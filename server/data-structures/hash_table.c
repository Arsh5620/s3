#include <string.h> 
#include <assert.h>
#include "hash_table.h"

/**
 * this function calculates a hash given the number and the modulus
 * the modulus must be a power of 2
 */
size_t hash_long(size_t number, size_t modulus)
{
    size_t c = ((number * 6654657629) ^ (number * 98767787)) + number;
	return c & (modulus - 1); 
}

// http://www.cse.yorku.ca/~oz/hash.html
unsigned long hash_string(char *memory, long len, unsigned long modulus)
{ 
    int c;
	unsigned long hash = 5381;
	char *addrlen	= memory + len;

    while (c = *memory++, memory < addrlen)
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash & (modulus-1);
}

size_t hash_hash(hash_input_u value, size_t len, char is_str, size_t mod)
{
    size_t hash  = 
		is_str 
		? hash_string(value.address, len, mod) 
		: hash_long(value.number, mod);
    return(hash);
}

/*
 * hash_table_init inits a new hash_table_s structure 
 * it allocates memory for HASH_TABLE_DEFAULT_SIZE buckets, 
 * and returns the structure which can later be used to fill the 
 * table and make use of it. 
 */
hash_table_s hash_table_init(long count, char is_string)
{
    hash_table_s table	= {0};
    table.size	= count;
    table.fill	= count * HASH_FILL;
    table.is_string	= is_string;
    size_t raw_size	= count * sizeof(hash_table_bucket_s);
    table.memory	= (hash_table_bucket_s*) calloc(raw_size, 1);
	assert(table.memory != NULL);
    return(table);
}

long hash_collision_count(hash_table_s table)
{
	return(table.collision_count);
}

/*
 * as first parameter this functions expects handle returned by 
 * hash_table_init(), and then it tries to add the "entry" to the 
 * hash table using open address hashing. 
 */
void hash_table_add(hash_table_s *table, hash_table_bucket_s entry)
{
    size_t hash = hash_hash(entry.key
		, entry.key_len, table->is_string, table->size);

    hash_table_bucket_s *dest	= (table->memory + hash);  
	if(dest->is_occupied)
		table->collision_count++;

    while (dest->is_occupied == HASH_OCCUPIED)
    {
        dest++;
        if(dest >= (table->memory + table->size))
            dest	= table->memory;
    }

    entry.is_occupied   = HASH_OCCUPIED;
    *dest = entry;
    table->count++;

    if(table->count >= table->fill)
        hash_table_expand(table);
}

// https://research.cs.vt.edu/AVresearch/hashing/deletion.php
void hash_table_remove(hash_table_s *table
	, hash_input_u key, size_t key_length)
{
    size_t hash = hash_hash(key, key_length, table->is_string, table->size);
    hash_table_bucket_s *addr	= (table->memory + hash);

	while(hash_compare(*addr, key, key_length, table->is_string)) {
        addr++;
        if(addr >= table->memory + table->size)
            addr = table->memory;
    }

	memset(addr, 0, sizeof(*addr));
	addr->is_occupied	= HASH_TOMBSTONE;
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
void hash_table_expand(hash_table_s *table)
{
	table->collision_count	= 0;
    size_t size	= table->size;

    table->count	= 0;
    table->size	*= HASH_EXPAND;
    table->fill	= table->size * HASH_FILL;

    size_t raw_size = table->size * sizeof(hash_table_bucket_s);
    hash_table_bucket_s *src  = table->memory, *src0	= src;
    table->memory  = (hash_table_bucket_s*) calloc(raw_size, 1);
	assert(table->memory != NULL);

	size_t i	= size;
    while (i--) {
		// we don't want to copy tombstoned buckets
        if (src->is_occupied == HASH_OCCUPIED)
            hash_table_add(table, *src);
        src++;
    }

    if(src0 != NULL)
        free(src0);
}

/* 
 * this function will check if we should continue looking into next bucket
 * when performing linear search
 */
char inline hash_compare(hash_table_bucket_s bucket
	, hash_input_u key, size_t key_len, char is_string)
{
	if (is_string == 0) {
		if(bucket.key.number == key.number)
			return(0);
	} else if (is_string == 1) {
		if(bucket.key.address == NULL
			|| memcmp(bucket.key.address, key.address, key_len) == 0)
			return(0);
	} else return(0);

// we can continue searching if the bucket is full or has a tombstone
	if(!bucket.is_occupied)
		return(0);
	return(1);
}

/*
 * hash_table_get is used to get the key:value pair from hash table
 * returns: hash_table_bucket_s
 */
hash_table_bucket_s hash_table_get(hash_table_s table
    , hash_input_u key, size_t key_length)
{    
    size_t hash = hash_hash(key, key_length, table.is_string, table.size);
    hash_table_bucket_s *addr	= (table.memory + hash)
		, result = {0};

	int i =0;
    while(i = hash_compare(*addr, key, key_length, table.is_string), i) {
        addr++;
        if(addr >= table.memory + table.size)
            addr = table.memory;
    }

    if(addr->is_occupied)
        result  = *addr;

    return result;
}

void hash_table_free(hash_table_s table) 
{
	if (table.memory) {
		free(table.memory);
	}
}