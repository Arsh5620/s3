#include "strings.h"

// int string_trim(string_internal_s *store);
// int string_forward_next_pair(string_internal_s *store);
// string_info_s string_count_accepted_chars(string_internal_s *store);
// key_value_pair_s string_next_key_value_pair(string_internal_s *store);

// // will process until the next new line either "\n" or "\r\n"
// // will consume until then and will move buffer pointer forward

// // the assignment operator can be either "="  or ":"
// // lvalue will be key, and rvalue will be value
// // quotes are not supported, underscore can be used instead
// // in place of " " (spaces)
// // you can have as many spaces as you like between lvalue, assignment
// // , and rvalue, everything will be trimmed.
// key_value_pair_s string_next_key_value_pair(string_internal_s *store)
// {
//     key_value_pair_s pair  = {0};

//     string_info_s key   = string_count_accepted_chars(store);
//     if(key.length > 0) {
//         pair.key    = key.buffer;
//         pair.key_length = key.length;
//     } else {
//         if(key.comment_break == 1)
//         {
//            pair.is_comment  = 1;
//         }
//         else
//             pair.is_valid   = 0;
//         return(pair);
//     }

//     string_trim(store);
//     if(*store->buffer_now == '=' || *store->buffer_now == ':')
//         store->buffer_now++;
//     else {
//         pair.is_valid = 0;
//         return(pair);
//     }
    
//     string_info_s value = string_count_accepted_chars(store);
//     if(value.length > 0){
//         pair.value  = value.buffer;
//         pair.value_length   = value.length;
//     } else {
//         pair.is_valid   = 0;
//         return(pair);
//     }

//     pair.is_valid   = 1;
//     return(pair);
// }

// int string_forward_next_pair(string_internal_s *store)
// {
//     char *buffer_start = store->buffer_now;

//     // first read the line until you reach a new line
//     while(*store->buffer_now++ != '\n'
//             && store->buffer_now < store->buffer_length);
//     // then read all the newlines until you get a non new line char. 
//     while(*store->buffer_now == '\n'
//             && store->buffer_now < store->buffer_length)
//         store->buffer_now++;

//     return(store->buffer_now - buffer_start);
// }

int strings_count_until(char *buffer, long length, char c)
{
    int count   = 0;
    while(*(buffer+count) != c && count < length)
        count++;
    return(count);
}

// string_info_s string_count_accepted_chars(string_internal_s *store)
// {
//     string_trim(store);
//     string_info_s info = {0};
//     info.buffer = store->buffer_now;
//     char *buffer_start  = store->buffer_now;
//     char break_loop = 0, escape = 0;
//     while(store->buffer_now < store->buffer_length && break_loop == 0){
//         char c  = *(store->buffer_now);

//         switch (c)
//         {
//         case '\"':
//         {
//             escape  = !escape;
//             if(escape){
//                 info.buffer++;
//             } else {
//                 break_loop  = 1;
//                 escape  = 1;
//             }
//             break;
//         }
//         case '_':
//         case '-':
//         case '.':
//         case '/':
//             break;

//         case '#':{
//             if (escape == 0){
//                 break_loop = 1;
//                 info.comment_break  = 1;
//             }
//             break;
//         }
//         default: 
//         {
//             if(escape){
//                 if(c != '\n')
//                 break;
//             } else {
//                 char c_nocaps   = c | 0b100000;
//                 if((c_nocaps >= 'a' && c_nocaps <='z')
//                     || (c >= '0' && c <= '9'))
//                     break; 
//             }
//             break_loop = 1;
//         }
//         }
//         if(break_loop == 0 || escape)
//             store->buffer_now++;
//     }
//     info.length = (store->buffer_now - info.buffer - escape);
//     return(info);
// }

// int string_trim(string_internal_s *store)
// {
//     char *buffer_start = store->buffer_now;
//     while(*store->buffer_now == ' '
//             && store->buffer_now < store->buffer_length) {
//         store->buffer_now++;
//     }

//     return(store->buffer_now - buffer_start);
// }

// array_list_s string_key_value_pairs(char *buffer, int length)
// {
//     string_internal_s store = {0};
//     store.buffer_now        = buffer;
//     store.buffer_address    = buffer;
//     store.buffer_length     = buffer + length;
//     store.list  = my_list_new(32, sizeof(key_value_pair_s));

//     while(store.buffer_now < store.buffer_length) {
//         key_value_pair_s pair = string_next_key_value_pair(&store);
//         if(pair.is_comment == 0 && pair.is_valid) {
//             my_list_push(&store.list, (void*)&pair);
//         }
//         string_forward_next_pair(&store);
//     }
//     return(store.list);
// }

void tolowercase(void *memory, int length)
{
    for(int i=0; i<length; ++i) {
        char c  = *(char*)(memory + i);
        *(char*)(memory + i) =  c >= 'A' && c <='Z' ? c | 32 : c;
    }
}

// int strings_next_newline(char *buffer, int length)
// {
//     int i   = 0;
//     while(i++ != length && *buffer != '\n');
//     if(i==length) return -1;
//     return(0);
// }

// function expects for your to have already called 
// file_reader_fill to fill the buffer with the data. 
// array_list_s strings_read_from_file(file_reader_s *reader)
// {
//     // check if within our given bounds we have a new line character
//     // if we don't have a new line character, we cannot process the string. 
//     return(string_key_value_pairs(reader->reader_malloc
//             , reader->reader_readlength));
// }