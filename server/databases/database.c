#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "database.h"
#include "../general/defines.h"
#include "../general/binarysearch.h"
#include "../memdbg/memory.h"

static MYSQL *sql_connection;

key_code_pair_s config_property[5] = {
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
    if(sql_connection != 0) return(DATABASE_SETUP_COMPLETE);

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
        fprintf(stderr, "%s\n", mysql_error(sql_connection));
        exit(SERVER_DATABASE_FAILURE);
    }
    return(DATABASE_SETUP_COMPLETE);
}

int database_verify_integrity()
{
    printf("db integrity check started ..\n");

    int connection_enabled  = mysql_ping(sql_connection);
    if(connection_enabled == 0){
        printf("welcome to server {%s}\n"
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
            printf("database \"%s\" found ..\n", DATABASE_DB_NAME);
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
    int query   = 
        mysql_query(sql_connection, DATABASE_TABLE_CHECKEXISTS1);

    MYSQL_RES *result = mysql_store_result(sql_connection);
    mysql_free_result(result); 
    // required to clear the session for next query

    if (query == MYSQL_SUCCESS) {
        printf("table \"" DATABASE_TABLE_NAME "\" also found ..\n");
    } else {
        printf("table \"" DATABASE_TABLE_NAME 
                    "\" not found, creating ..\n");
        return(database_create_tables());
    }
    return(MYSQL_SUCCESS);
}

void database_set_isnull(char flags, my_bool *is_null)
{
    if(!(flags & DATETIME_CREATE_DATE))
        is_null[TABLE1_INDEX_FILE_CD] = 1;
    if(!(flags & DATETIME_UPLOAD_DATE))
        is_null[TABLE1_INDEX_FILE_UD] = 1;
    if(!(flags & DATETIME_LASTACCESS_DATE))
        is_null[TABLE1_INDEX_FILE_LA] = 1;
    if(!(flags & DATETIME_LASTCHANGE_DATE))
        is_null[TABLE1_INDEX_FILE_LM] = 1;
    if(!(flags & DATETIME_DELETION_DATE))
        is_null[TABLE1_INDEX_FILE_DD] = 1;
}

// this function and the accompanying free functions, 
// help with allocating memory for either pushing or retrieving
// records from the table name #define DATABASE_TABLE_NAME
//
// ** you don't need to use this function to init the struct
// ** it is just a helper routine, but you would need to call
// ** the database_table1_free to make sure all memory gets
// ** deallocated if you so decide to use this function
database_table1_s *database_table1_allocate()
{
    database_table1_s *table =
        m_calloc(sizeof(database_table1_s), MEMORY_FILE_LINE);

    table->folder_name.address = 
    m_calloc(DATABASE_FOLDERNAME_LEN, MEMORY_FILE_LINE);
    table->folder_name.length = DATABASE_FOLDERNAME_LEN;

    table->file_name.address = 
    m_calloc(DATABASE_FILENAME_LEN, MEMORY_FILE_LINE);
    table->file_name.length = DATABASE_FILENAME_LEN;

    return(table);
}

// frees memory allocated by database_table1_allocate
void database_table1_free(database_table1_s *table)
{
    if(table && table->file_name.address)
        free(table->file_name.address);

    if(table && table->folder_name.address)
        free(table->folder_name.address);

    if (table) 
        m_free(table, MEMORY_FILE_LINE);
}

// will just call m_free on the mallocated memory area
// the structure should not be used after this call. 
void database_table1_bind_free(db_table_stmt_s *table)
{
    m_free(table, MEMORY_FILE_LINE);
}
db_table_stmt_s *database_table1_bind_get(database_table1_s *table)
{
    int select[DATABASE_TABLE1_COLUMNCOUNT] = {
        TABLE1_INDEX_FOLDER_NAME  
        , TABLE1_INDEX_FILE_NAME  
        , TABLE1_INDEX_FILE_CD
        , TABLE1_INDEX_FILE_UD
        , TABLE1_INDEX_FILE_LA
        , TABLE1_INDEX_FILE_LM
        , TABLE1_INDEX_FILE_DD
        , TABLE1_INDEX_FILE_SIZE  
        , TABLE1_INDEX_FILE_MD5
        , TABLE1_INDEX_PERMISSIONS
        , TABLE1_INDEX_OWNER
    };
    return (database_table1_bind_getselective(table
        , select, DATABASE_TABLE1_COLUMNCOUNT));
}

// caller is responsible to call m_free when structure not in use. 
// primary purpose of this function is to setup the database_table1_s
// for execution before calling database_insert_table1() or likes
// please make sure to fill the "table" before calling this function. 
db_table_stmt_s *database_table1_bind_getselective(
    database_table1_s *table
    , int *select
    , int count)
{
    db_table_stmt_s *stmt_table1 =  
        m_calloc(sizeof(db_table_stmt_s), MEMORY_FILE_LINE);

    enum enum_field_types types[DATABASE_TABLE1_COLUMNCOUNT] = {
        MYSQL_TYPE_STRING
        , MYSQL_TYPE_STRING
        , MYSQL_TYPE_TIME
        , MYSQL_TYPE_TIME
        , MYSQL_TYPE_TIME
        , MYSQL_TYPE_TIME
        , MYSQL_TYPE_TIME
        , MYSQL_TYPE_LONG
        , MYSQL_TYPE_STRING
        , MYSQL_TYPE_LONG
        , MYSQL_TYPE_STRING
    };
    memcpy(stmt_table1->types, types, sizeof(types));
    
    stmt_table1->length[TABLE1_INDEX_FILE_MD5]   = sizeof(table->file_md5);
    stmt_table1->length[TABLE1_INDEX_OWNER]   = sizeof(table->owner);

    unsigned long int *length_ptrs[DATABASE_TABLE1_COLUMNCOUNT] = {
        &table->folder_name.length // string
        , &table->file_name.length // string
        , &stmt_table1->length[TABLE1_INDEX_FILE_CD] // datetime struct
        , &stmt_table1->length[TABLE1_INDEX_FILE_UD] // datetime struct
        , &stmt_table1->length[TABLE1_INDEX_FILE_LA] // datetime struct
        , &stmt_table1->length[TABLE1_INDEX_FILE_LM] // datetime struct
        , &stmt_table1->length[TABLE1_INDEX_FILE_DD] // datetime struct
        , &stmt_table1->length[TABLE1_INDEX_FILE_SIZE] // long
        , &stmt_table1->length[TABLE1_INDEX_FILE_MD5] // string
        , &stmt_table1->length[TABLE1_INDEX_PERMISSIONS] // long
        , &stmt_table1->length[TABLE1_INDEX_OWNER] // string
    };
    memcpy(stmt_table1->length_ptrs, length_ptrs, sizeof(length_ptrs));

    char *buffers[DATABASE_TABLE1_COLUMNCOUNT]  = {
        (char*)table->folder_name.address
        , (char*)table->file_name.address
        , (char*)&table->file_cd
        , (char*)&table->file_ud
        , (char*)&table->file_la
        , (char*)&table->file_lm
        , (char*)&table->file_dd
        , (char*)&table->file_size
        , (char*)table->file_md5
        , (char*)&table->permissions
        , (char*)table->owner
    };
    memcpy(stmt_table1->buffers, buffers, sizeof(buffers));

    database_set_isnull(table->file_times_set, stmt_table1->is_null);

    for(int i=0; i < count; ++i) {

        int order   = select[i];
        // NOTE to self: 
        // x[i] is same as *(x + i)
        // while x + i is (address) + ((sizeof(typeof(x))) * i)
        database_bind_param(stmt_table1->bind_params + i
            , stmt_table1->types[order]
            , stmt_table1->buffers[order]
            , stmt_table1->length_ptrs[order]
            , stmt_table1->is_null + order
            , stmt_table1->is_error + order);
        if(stmt_table1->types[1] == MYSQL_TYPE_STRING)
            stmt_table1->bind_params[order].buffer_length   = 
                *stmt_table1->length_ptrs[order];
    }
    return(stmt_table1);
}

int database_table1_insert(
    database_table1_s table
    , db_table_stmt_s * table1)
{
    MYSQL_STMT *stmt = mysql_stmt_init(sql_connection);
    if(stmt == NULL)
        return(MYSQL_ERROR);

    int result  = mysql_stmt_prepare(stmt, DATABASE_TABLE1_INSERT
                                        , strlen(DATABASE_TABLE1_INSERT));
    if(result){
        mysql_stmt_close(stmt);
        return(MYSQL_ERROR);
    }
  
    result  = mysql_stmt_bind_param(stmt, table1->bind_params);
    if(result){
        mysql_stmt_close(stmt);
        return(MYSQL_ERROR);
    }

    result  = mysql_stmt_execute(stmt);
    if(result){
        mysql_stmt_close(stmt);
        return(MYSQL_ERROR);
    }
    
    return(mysql_affected_rows(sql_connection) > 0);
}

int database_table_query_results(MYSQL_STMT *statment)
{
    int result  = mysql_stmt_fetch(statment);
    if(result){
        mysql_stmt_close(statment);
        return(MYSQL_ERROR);
    }
    return(MYSQL_SUCCESS);
}

/**
 * "this function should not be used in production or non-debug builds"
 * should only be used to print all out the records returned
 * by running a sql select like statement
 */
void __database_query_print_dbg(MYSQL_STMT *stmt, db_table_stmt_s *binds)
{   
    MYSQL_RES *result   = mysql_stmt_result_metadata(stmt);
    if(result == NULL)
        printf("result from the statment is null @%s:%d\n"
                , __FILE__, __LINE__);
    int num_columns = mysql_num_fields(result);

    while (mysql_stmt_fetch(stmt) == 0)
    {
        printf("%s, %s, %s, %ld, %ld.\n"
                , binds->buffers[TABLE1_INDEX_FOLDER_NAME]
                , binds->buffers[TABLE1_INDEX_FILE_NAME]
                , binds->buffers[TABLE1_INDEX_OWNER]
                , *(long int*)binds->buffers[TABLE1_INDEX_FILE_SIZE]
                , *(long int*)binds->buffers[TABLE1_INDEX_PERMISSIONS]);
    }
}

MYSQL_STMT *database_table1_query(db_table_stmt_s *binds
    , char *query, MYSQL_BIND *in_bind)
{
    MYSQL_STMT *stmt    = mysql_stmt_init(sql_connection);
    if(stmt == NULL)
        return(NULL);
    
    // as per the example on mariadb docs, -1 means strlen internally
    int result  = mysql_stmt_prepare(stmt, query, -1);
    if(result){
        printf("sql could not finish, error: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return(NULL);
    }
    result  = mysql_stmt_bind_result(stmt, binds->bind_params);
    if(result){
        mysql_stmt_close(stmt);
        return(NULL);
    }
    result  = mysql_stmt_bind_param(stmt, in_bind);
    if(result){
        mysql_stmt_close(stmt);
        return(NULL);
    }
    result  = mysql_stmt_execute(stmt);
    if(result){
        mysql_stmt_close(stmt);
        return(NULL);
    }
    return(stmt);
}

void database_bind_param(MYSQL_BIND *bind
    , enum enum_field_types type
    , char *buffer_pointer
    , unsigned long int *length
    , my_bool *is_null
    , my_bool *error)
{
  bind->buffer_type  = type;
  bind->buffer   = buffer_pointer;
  bind->is_null  = is_null;
  bind->length   = length;
  bind->error    = error;
}