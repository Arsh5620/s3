# ifndef STRINGS_INCLUDE_GAURD
# define STRINGS_INCLUDE_GAURD

// #include "list.h"
// #include "file.h"

// typedef struct buffer_information {
//     char *buffer;
//     int length;
//     char comment_break;
// } string_info_s;
 
 typedef struct string_struct {
    void *address;
    unsigned long int length;
    int error;
} string_s;

// typedef struct string_process_internal {
//     array_list_s list;
//     char *buffer_address;
//     char *buffer_now;
//     char *buffer_length;

//     char eof;
// } string_internal_s;

// typedef struct {
//     char *key;
//     char *value;

//     int key_length;
//     int value_length;

//     char is_valid;
//     char is_comment;
// } key_value_pair_s;

void tolowercase(void *memory, int length);
int strings_count_until(char *buffer, long length, char c);

#endif