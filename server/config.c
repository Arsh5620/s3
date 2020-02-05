#include "config.h"
#include <string.h>
#include <stdlib.h>

// config_read_file must read a file interpret it and return 
// key:value pairs that can be used to initialize the program
array_list_s config_read_file(FILE *file)
{
    file_reader_s reader    = file_init_reader(file);

    int result  = file_reader_fill(&reader);

    array_list_s parsed_table   = strings_read_from_file(&reader);
    return(parsed_table);
}

database_connection_s config_parse_dbc(char *filename)
{
    FILE *config    = fopen(filename, FILE_MODE_READONLY);

    database_connection_s connection = {0};
    array_list_s list = config_read_file(config);

    for(int i=0; i<list.index; ++i){
        key_value_pair_s pair = *(key_value_pair_s*) list_get(list, i);
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
        }
    }

    fclose(config);
    return(connection);
}   
