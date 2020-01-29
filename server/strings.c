#include "strings.h"

int string_trim(string_internal_s *store);
int string_forward_next_pair(string_internal_s *store);
string_info_s string_count_accepted_chars(string_internal_s *store);
key_value_pair_s string_next_key_value_pair(string_internal_s *store);

// will process until the next new line either "\n" or "\r\n"
// will consume until then and will move buffer pointer forward

// the assignment operator can be either "="  or ":"
// lvalue will be key, and rvalue will be value
// quotes are not supported, underscore can be used instead
// in place of " " (spaces)
// you can have as many spaces as you like between lvalue, assignment
// , and rvalue, everything will be trimmed.  
key_value_pair_s string_next_key_value_pair(string_internal_s *store)
{
    key_value_pair_s pair  = {0};

    string_info_s key   = string_count_accepted_chars(store);
    if(key.length > 0) {
        pair.key    = key.buffer;
        pair.key_length = key.length;
    } else {
        pair.is_valid   = 0;
        return(pair);
    }

    string_trim(store);
    if(*store->buffer_now == '=' || *store->buffer_now == ':')
        store->buffer_now++;
    else {
        pair.is_valid = 0;
        return(pair);
    }
    
    string_info_s value = string_count_accepted_chars(store);
    if(value.length > 0){
        pair.value  = value.buffer;
        pair.value_length   = value.length;
    } else {
        pair.is_valid   = 0;
        return(pair);
    }

    string_forward_next_pair(store);
    pair.is_valid   = 1;
    return(pair);
}

int string_forward_next_pair(string_internal_s *store)
{
    char *buffer_start = store->buffer_now;
    while(*store->buffer_now++ != '\n'
            && store->buffer_now < store->buffer_length);

    return(store->buffer_now - buffer_start);
}

string_info_s string_count_accepted_chars(string_internal_s *store)
{
    string_trim(store);
    string_info_s info = {0};
    info.buffer = store->buffer_now;
    char *buffer_start  = store->buffer_now;
    char break_loop = 0;
    while(store->buffer_now < store->buffer_length && break_loop == 0){
        char c  = *(store->buffer_now);

        switch (c)
        {
        case '_':
        case '-':
        case '.':
        case '/':
            break;;
        default: 
        {
            char c_nocaps   = c | 0b100000;
            if((c_nocaps >= 'a' && c_nocaps <='z')
                || (c >= '0' && c <= '9'))
                break;
            
            break_loop = 1;
        }
        }
        if(break_loop == 0)
            store->buffer_now++;
    }
    info.length = (store->buffer_now - buffer_start);
    return(info);
}

int string_trim(string_internal_s *store)
{
    char *buffer_start = store->buffer_now;
    while(*store->buffer_now == ' '
            && store->buffer_now < store->buffer_length) {
        store->buffer_now++;
    }

    return(store->buffer_now - buffer_start);
}

array_list_s string_key_value_pairs(char *buffer, int length)
{
    string_internal_s store = {0};
    store.buffer_now        = buffer;
    store.buffer_address    = buffer;
    store.buffer_length     = buffer + length;
    store.list  = list_new(32, sizeof(key_value_pair_s));

    while(store.eof != 1) {
        key_value_pair_s pair = string_next_key_value_pair(&store);
        if(pair.is_valid == 0)
            store.eof   = 1;
        else {
            list_push(&store.list, (void*)&pair);
        }
    }
    return(store.list);
}

void tolowercase(void *memory, int length)
{
    for(int i=0; i<length; ++i) {
        char c  = *(char*)(memory + i);
        *(char*)(memory + i) =  c >= 'A' && c <='Z' ? c | 32 : c;
    }
}