# ifndef STRINGS_INCLUDE_GAURD
# define STRINGS_INCLUDE_GAURD

#include "list.h"
#include "file.h"

typedef struct buffer_information {
    char *buffer;
    int length;
    char comment_break;
} string_info_s;
 
 typedef struct string_struct {
    void *address;
    unsigned long int length;
    int error;
} string_s;

typedef struct key_value_pair {
    char *key;
    char *value;

    int key_length;
    int value_length;

    char is_valid;
    char is_comment;
} key_value_pair_s;

typedef struct string_process_internal {
    array_list_s list;
    char *buffer_address;
    char *buffer_now;
    char *buffer_length;

    char eof;
} string_internal_s;

void tolowercase(void *memory, int length);
array_list_s string_key_value_pairs(char *buffer, int length);
array_list_s strings_read_from_file(file_reader_s *reader);
#endif