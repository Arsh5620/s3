#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dbp.h"
#include "memory.h"
#include "parser/parser.h"
#include "protocol/protocol.h"
#include "binarysearch.h"

// static b_search_string_s actions[] = 
// {
//     {"create", 6, .code = ACTION_CREATE}
//     , {"notification", 12, .code = ACTION_NOTIFICATION} 
//     , {"request", 7, .code = ACTION_REQUEST}
//     , {"update", 6, .code = ACTION_UPDATE}
// };


int main(int argc, char *argv[])
{
    // array_list_s list = my_list_new(1, sizeof(key_value_pair_s));
    // key_value_pair_s pair = {.key = "create", .key_length = 6, .value = "how", .value_length = 3};
    // my_list_push(&list, (char*)&pair);
    // key_value_pair_s pair2 = {.key = "notification", .key_length = 12, .value = "how", .value_length = 3};
    // my_list_push(&list, (char*)&pair2);
    // key_value_pair_s pair3 = {.key = "request", .key_length = 7, .value = "how", .value_length = 3};
    // my_list_push(&list, (char*)&pair3);
    // key_value_pair_s pair4 = {.key = "update", .key_length = 6, .value = "how", .value_length = 3};
    // my_list_push(&list, (char*)&pair4);

    // int match[] = {ACTION_CREATE, ACTION_REQUEST, ACTION_UPDATE};
    // int result  = dbp_assert_list(list, actions, sizeof(actions)/sizeof(b_search_string_s), 
    // match, sizeof(match)/sizeof(int));
    // printf("result is %d\n", result);

    dbp_s protocol  = dbp_init(APPLICATION_PORT);
    if(protocol.is_init)
        dbp_accept_connection_loop(&protocol);
    dbp_cleanup(protocol);
}