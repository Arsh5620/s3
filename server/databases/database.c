#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "database.h"
#include "../defines.h"
#include "../binarysearch.h"

static MYSQL *sql_connection;

b_search_string_s config_property[5] = {
    {"database", 8, CONFIG_DATABASE}
    , {"machine", 7, CONFIG_MACHINE}
    , {"password", 8, CONFIG_PASSWORD}
    , {"port", 4, CONFIG_PORT}
    , {"username", 8, CONFIG_USERNAME}
};


// after database_init is called a static variable sql_connection
// is initialized to the connection information and a mysql db 
// connection is attempted. 
int database_init(database_connection_s connect)
{
    if (mysql_library_init(0, NULL, NULL)) {
        fprintf(stderr, "could not initialize MySQL client library.\n");
        exit(SERVER_DATABASE_FAILURE);
    }
    sql_connection  = mysql_init(0);
    if (sql_connection == NULL) {
        fprintf(stderr, "could not initialize the mysql"
                    " connection, exiting..\n");
        exit(SERVER_DATABASE_FAILURE);
    }

    if (NULL == mysql_real_connect(sql_connection
        , connect.host , connect.user
        , connect.passwd , connect.db
        , connect.port , NULL , 0)) {
        fprintf(stderr, "could not connect to mariaDB server"
            ", check your connection settings and try again.\n");
        exit(SERVER_DATABASE_FAILURE);
    }

    return(DATABASE_SETUP_COMPLETE);
}

int database_verify_integrity()
{
    printf("db integrity check started ..\n");

    int connection_enabled  = mysql_ping(sql_connection);
    if(connection_enabled == 0){
        printf("connection to the server was successful ..\n");
        printf("server version %s\n"
                , mysql_get_server_info(sql_connection));
    }
    if(MYSQL_SUCCESS == mysql_query(sql_connection
        , "CREATE DATABASE IF NOT EXISTS " DATABASE_DB_NAME))
    {
        size_t  db_created  = mysql_affected_rows(sql_connection);

        if(MYSQL_SUCCESS != 
                mysql_select_db(sql_connection, DATABASE_DB_NAME)) {
            printf("failed to select the correct database ..");
            return(MYSQL_ERROR);
        }

        if(db_created){
            printf("database not found,"
                    " created database \""DATABASE_DB_NAME"\" ..\n");
            return(database_create_tables());
        } else {
            printf("database found ..\n");
            return(database_check_tables());
        }
    } else {
        printf("checking database failed, \"CREATE\""
                " access required for the user\n");
    }
    return(MYSQL_ERROR);
}

int database_create_tables()
{
    if (mysql_query(sql_connection, DATABASE_TABLE_1) == MYSQL_SUCCESS) {
        printf("table " DATABASE_TABLE_NAME " created ..\n");
    } else {
        fprintf(stderr, "table \"" DATABASE_TABLE_NAME 
        "\" could not be created ..\n");
        fprintf(stderr, "Error: %s\n", mysql_error(sql_connection));
        return(MYSQL_ERROR);
    }
    return(MYSQL_SUCCESS);
}

int database_check_tables()
{
    if (mysql_query(sql_connection, DATABASE_TABLE_CHECKEXISTS1) 
            == MYSQL_SUCCESS) {
        printf("table \"" DATABASE_TABLE_NAME "\" also found ..\n");
    } else {
        printf("table \"" DATABASE_TABLE_NAME 
                    "\" not found, creating ..\n");
        return(database_create_tables());
    }
    return(MYSQL_SUCCESS);
}

int database_insert_table1(database_table1_s table)
{
    MYSQL_STMT *stmt = mysql_stmt_init(sql_connection);
    MYSQL_BIND bind_params[11]  = {0};
    my_bool is_null[11] = {0};
    my_bool is_error[11] = {0};
    
    int result  = mysql_stmt_prepare(stmt, DATABASE_TABLE1_INSERT
                                        , strlen(DATABASE_TABLE1_INSERT));

    //folder name
    database_bind_param(&bind_params[0]
        , MYSQL_TYPE_VARCHAR
        , table.folder_name.address
        , (unsigned long*) &table.folder_name.length
        , &is_null[0]
        , &is_error[0]);

    //file name
    database_bind_param(&bind_params[1]
        , MYSQL_TYPE_VARCHAR
        , table.folder_name.address
        , (unsigned long*) &table.folder_name.length
        , &is_null[1]
        , &is_error[1]);

    //all date and time values.
    MYSQL_TIME *times[5] = {
        &table.file_md
        , &table.file_ud
        , &table.file_la
        , &table.file_lm
        , &table.file_dd
    };
    unsigned long int lengths[5] = {0};

    database_bind_multiple(&bind_params[3], 5
        , MYSQL_TYPE_TIME, (char**)&times
        , lengths + 3, is_null + 3, is_error + 3);

     database_bind_param(&bind_params[8]
        , MYSQL_TYPE_LONG
        , (char*)&table.file_size
        , 0
        , &is_null[8]
        , &is_error[8]);

    unsigned long int md5len  = sizeof(table.file_md5);
    database_bind_param(&bind_params[9]
        , MYSQL_TYPE_VARCHAR
        , (char*)table.file_md5
        , &md5len
        , &is_null[9]
        , &is_error[9]);

    database_bind_param(&bind_params[10]
        , MYSQL_TYPE_LONG
        , (char*)&table.permissions
        , 0
        , &is_null[10]
        , &is_error[10]);

    unsigned long int ownerlen  = sizeof(table.owner);
    database_bind_param(&bind_params[11]
        , MYSQL_TYPE_VARCHAR
        , table.owner
        , &ownerlen
        , &is_null[11]
        , &is_error[11]);

    return(mysql_stmt_execute(stmt));
}

void database_bind_multiple(MYSQL_BIND *bind_start_address
    , int count
    , enum enum_field_types type
    , char **buffer_pp
    , unsigned long int *length
    , my_bool *is_null
    , my_bool *error)
{
    for(int i=0; i<count; ++i) {
        database_bind_param(&bind_start_address[i]
            , type
            , buffer_pp[i]
            , &length[i]
            , &is_null[i]
            , &error[i]);
    }
}

void database_bind_param(MYSQL_BIND *bind
    , enum enum_field_types type
    , char *buffer_pointer
    , unsigned long int *length
    , my_bool *is_null
    , my_bool *error)
{
  if(buffer_pointer == 0)
    *is_null    = 1;

  bind->buffer_type  = type;
  bind->buffer   = buffer_pointer;
  bind->is_null  = is_null;
  bind->length   = length;
  bind->error    = error;
}