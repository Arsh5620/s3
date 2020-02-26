#include "config.h"
#include <string.h>
#include <stdlib.h>

database_connection_s config_parse_dbc(char *filename)
{
    FILE *config    = fopen(filename, FILE_MODE_READONLY);

    database_connection_s connection = {0};
    array_list_s list = parser_parse_file(config);

    for(int i=0; i<list.index; ++i){
        key_value_pair_s pair = *(key_value_pair_s*) my_list_get(list, i);
        int configcode  = 
            b_search(config_property
                , sizeof(config_property)/ sizeof(b_search_string_s)
                , pair.key, pair.key_length);

        switch (config_property[configcode].code)
        {
        case CONFIG_DATABASE:
            memcpy((void*)connection.db
                    , (void*)pair.value, pair.value_length);
            break;
        case CONFIG_USERNAME:
            memcpy((void*)connection.user
                    , (void*)pair.value, pair.value_length);
            break;
        case CONFIG_PASSWORD:
            memcpy((void*)connection.passwd
                    , (void*)pair.value, pair.value_length);
            break;
        case CONFIG_MACHINE:
            memcpy((void*)connection.host
                    , (void*)pair.value, pair.value_length);
            break;
        case CONFIG_PORT:
            connection.port = (int) strtol(pair.value, NULL, 10);
            break;
        default:
            // do nothing if we have a key:value pair that we don't need
            break;
        }
    }

    fclose(config);
    parser_release_list(list);
    return(connection);
}   
