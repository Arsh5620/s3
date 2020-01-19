#include "hash_table.h"
#include "malloc.h"
#include "./networking/defines.h"

/**
 * in - the index or the key to use. 
 * modulus - the highest value or ceil (must be multiple of 2)
 */
unsigned int hash_table_hash(unsigned long in, unsigned int modulus)
{
    unsigned int _a1    = in & 0xFFFFFFFF
                , _a2   = (~in >> 32) & 0xFFFFFFFF;
    unsigned long li = (((_a1 * 2726656) ^ (_a2 * 9931)) + _a2);
    li += ~li / modulus;
	return li & (modulus - 1);
}

hash_table_s hash_table_init()
{
    hash_table_s hash_table = {0};
    hash_table.length   = HASH_TABLE_DEFAULT_SIZE;
    hash_table.raw_size = hash_table.length * sizeof(hash_table_entry_s);
    hash_table.fill_factor  = hash_table.length * HASH_TABLE_FILL_FACTOR;

    hash_table.hash_memory  = (hash_table_entry_s*)calloc(hash_table.raw_size, 1);

    return(hash_table);
}

int hash_table_add(hash_table_s *table, hash_table_entry_s entry)
{
    hash_table_entry_s *destination = 
        &table->hash_memory[hash_table_hash(entry.key, table->length)];
    
    while (destination->_bucket_occupied == TRUE)
    {
        ++destination;

        if(destination >= (table->hash_memory + table->length))
            destination = table->hash_memory;
    }

    entry._bucket_occupied   = TRUE;
    *destination = entry;

    table->index++;

    if(table->index >= table->fill_factor)
        hash_table_expand(table);

    return (TRUE);
}

hash_table_s hash_table_expand(hash_table_s *table)
{
    unsigned int original_size  = table->length;

    table->index            = 0;
    table->length           *= HASH_TABLE_EXPAND_SIZE;
    table->raw_size         = table->length * (sizeof(hash_table_entry_s));
    table->fill_factor      = table->length * HASH_TABLE_FILL_FACTOR;

    hash_table_entry_s *source  = table->hash_memory, *source2 = source;
    table->hash_memory  = (hash_table_entry_s*)calloc(table->raw_size, 1);

    while(original_size--) {
        if(source->_bucket_occupied)
            hash_table_add(table, *source);
        
        source++;
    }

    if(source2 != NULL)
        free(source2);

    return(*table);
}

hash_table_entry_s hash_table_get(hash_table_s table, unsigned long key)
{
    unsigned int hash   = hash_table_hash(key, table.length);
    hash_table_entry_s *entry = &table.hash_memory[hash];
    hash_table_entry_s result = {0};

    while(entry->key != key && entry->_bucket_occupied) {
        entry++;
        if(entry >= table.hash_memory + table.length)
            entry = table.hash_memory;
    }

    if(entry->key == key)
        result  = *entry;

    return result;
}