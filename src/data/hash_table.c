#include "hash_table.h"
#include "general/common.h"

#include <assert.h>
#include <stddef.h>
#include <string.h>

void
hash_table_expand (hash_table_s *table);
int
hash_compare (hash_table_bucket_s bucket, char *key, long key_length);

/**
 * This function calculates a hash given the number and the modulus
 * The modulus must be a power of 2
 */
size_t
hash_long (size_t number, size_t modulus)
{
    size_t c = ((number * 6654657629) ^ (number * 98767787)) + number;
    return c & (modulus - 1);
}

// http://www.cse.yorku.ca/~oz/hash.html
unsigned long
hash_string (char *memory, long len, unsigned long modulus)
{
    int c;
    unsigned long hash = 5381;
    char *addrlen = memory + len;

    while (c = *memory++, memory < addrlen)
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash & (modulus - 1);
}

size_t
hash_hash (char *key, long length, long modulus)
{
    if (length == 8) {
        return hash_long (*(size_t *) key, modulus);
    } else {
        return hash_string (key, length, modulus);
    }
}

/*
 * Creates a new hash_table_s structure, allocates memory for intial_count buckets
 */
hash_table_s
hash_table_alloc (long initial_count, float fill_percentage, float expand_multiply, short expand_add)
{
    hash_table_s table = {0};
    table.capacity = initial_count;
    table.capacity_max_fill = initial_count * fill_percentage;
    table.hash_fill_multiply = fill_percentage;
    table.expand_multiply = expand_multiply;
    table.expand_add = expand_add;

    size_t raw_size = initial_count * sizeof (hash_table_bucket_s);
    table.memory = calloc (raw_size, 1);
    assert (table.memory != NULL);

    return table;
}

void
hash_table_add (hash_table_s *table, char *key, long key_length, char *value, long value_length)
{
    size_t hash = hash_hash (key, key_length, table->capacity);
    hash_table_bucket_s *table_buckets = table->memory + hash;

    if (table_buckets->occupied) {
        table->collision_count++;
    }

    // Find first place that isn't occupied, this will use the first place that is tombstoned
    // or the first place that is empty
    while (table_buckets->occupied == HASH_OCCUPIED) {
        table_buckets++;
        if (table_buckets >= table->memory + table->capacity) {
            table_buckets = table->memory;
        }
    }

    hash_table_bucket_s *bucket = table_buckets;

    bucket->occupied = HASH_OCCUPIED;
    bucket->key = key;
    bucket->value = value;
    bucket->key_length = key_length;
    bucket->value_length = value_length;

    table->count++;

    if (table->count > table->capacity_max_fill) {
        hash_table_expand (table);
    }
}

// https://research.cs.vt.edu/AVresearch/hashing/deletion.php
void
hash_table_remove (hash_table_s *table, char *key, long key_length)
{
    size_t hash = hash_hash (key, key_length, table->capacity);
    hash_table_bucket_s *table_buckets = table->memory + hash;

    while (hash_compare (*table_buckets, key, key_length)) {
        table_buckets++;
        if (table_buckets >= table->memory + table->capacity) {
            table_buckets = table->memory;
        }
    }

    memset (table_buckets, 0, sizeof *table_buckets);
    table_buckets->occupied = HASH_TOMBSTONE;
}

// This function is extremely expensive as it copies over all of the memory
// when expanding the size of the hash table
void
hash_table_expand (hash_table_s *table)
{
    table->collision_count = 0;
    size_t size = table->capacity;

    table->count = 0;
    table->capacity = (table->capacity * table->expand_multiply) + table->expand_add;
    table->capacity_max_fill = table->capacity * table->hash_fill_multiply;

    size_t raw_size = table->capacity * sizeof (hash_table_bucket_s);
    hash_table_bucket_s *source_memory = table->memory, *source_memory_cpy = source_memory;

    table->memory = calloc (raw_size, 1);
    assert (table->memory != NULL);

    size_t i = size;
    while (i--) {
        // Copy all of the buckets except tombstoned
        if (source_memory->occupied == HASH_OCCUPIED) {
            hash_table_bucket_s bucket = *source_memory;
            hash_table_add (table, bucket.key, bucket.key_length, bucket.value, bucket.value_length);
        }
        source_memory++;
    }

    if (source_memory_cpy) {
        free (source_memory_cpy);
    }
}

/*
 * @return boolean bucket != key AND no more values to iterate
 */
int
hash_compare (hash_table_bucket_s bucket, char *key, long key_length)
{
    if (bucket.occupied == 0) {
        return FALSE;
    }

    if (bucket.occupied == HASH_TOMBSTONE) {
        return TRUE;
    }

    if (bucket.key_length != key_length) {
        return TRUE;
    }

    int cmp = memcmp (bucket.key, key, key_length);
    if (cmp == 0) { // bucket == key
        return FALSE;
    }

    return FALSE;
}

/*
 * hash_table_get is used to get the key:value pair from hash table
 * returns: hash_table_bucket_s
 */
hash_table_bucket_s
hash_table_get (hash_table_s table, char *key, long key_length)
{
    size_t hash = hash_hash (key, key_length, table.capacity);
    hash_table_bucket_s *table_buckets = table.memory + hash, result = {0};

    while (hash_compare (*table_buckets, key, key_length)) {
        table_buckets++;
        if (table_buckets >= table.memory + table.capacity) {
            table_buckets = table.memory;
        }
    }

    if (table_buckets->occupied) {
        result = *table_buckets;
    }

    return result;
}

void
hash_table_free (hash_table_s table)
{
    if (table.memory) {
        free (table.memory);
        table.memory = NULL;
    }
}