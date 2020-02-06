#include <mysql.h>
#include <stdio.h>
#include <stdlib.h>
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