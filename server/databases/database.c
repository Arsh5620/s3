#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "database.h"
#include "../logger/logs.h"
#include "../general/defines.h"
#include "../general/binarysearch.h"
#include "../memdbg/memory.h"
#include "../errors/errorhandler.h"

static MYSQL *sql_connection;
static database_table_bind_s binds;

MYSQL *database_get_handle()
{
	return(sql_connection);
}
// after database_init is called a static variable sql_connection
// is initialized to the connection information and a mysql db 
// connection is attempted. 
int database_init(database_connection_s connect)
{
    if(sql_connection != 0) return(DATABASE_SETUP_COMPLETE);

    if (mysql_library_init(0, NULL, NULL)) {
		error_handle(ERRORS_HANDLE_LOGS, LOGGER_ERROR
			, DATABASE_MYSQL_LIB_INIT_FAILED);
        exit(SERVER_DATABASE_FAILURE);
    }
    sql_connection  = mysql_init(0);
    if (sql_connection == NULL) {
        error_handle(ERRORS_HANDLE_LOGS, LOGGER_ERROR
			, DATABASE_MYSQL_INIT_FAILED);
        exit(SERVER_DATABASE_FAILURE);
    }

    if (NULL == mysql_real_connect(sql_connection
        , connect.host , connect.user
        , connect.passwd , connect.db
        , connect.port , NULL , 0)) {
       	error_handle(ERRORS_HANDLE_LOGS, LOGGER_ERROR
			, DATABASE_MYSQL_AUTH_FAILED_S
			, mysql_error(sql_connection));
        exit(SERVER_DATABASE_FAILURE);
    }
    error_handle(ERRORS_HANDLE_LOGS, LOGGER_INFO
			, DATABASE_MYSQL_CONNECTED);

    return(DATABASE_SETUP_COMPLETE);
}

int database_verify_integrity()
{
    error_handle(ERRORS_HANDLE_LOGS, LOGGER_INFO
			, DATABASE_INTEGRITY_CHECK);
	error_handle(ERRORS_HANDLE_LOGS, LOGGER_INFO
			, DATABASE_INTEGRITY_PING);

    int connection_enabled  = mysql_ping(sql_connection);
    if(connection_enabled == 0) { 
		error_handle(ERRORS_HANDLE_LOGS, LOGGER_INFO
			, DATABASE_CONNECTED_SERVER_S
			, mysql_get_server_info(sql_connection));
    }

    if(MYSQL_SUCCESS == mysql_query(sql_connection
        , DATABASE_CREATE_DATABASE))
    {
        size_t  db_created  = mysql_affected_rows(sql_connection);
        if(MYSQL_SUCCESS != 
                mysql_select_db(sql_connection, DATABASE_DB_NAME)) {
			error_handle(ERRORS_HANDLE_LOGS, LOGGER_ERROR
				, DATABASE_INT_DB_SELECT_FAILED_S
				, mysql_error(sql_connection));
            return(MYSQL_ERROR);
        }

        if(db_created){
            error_handle(ERRORS_HANDLE_LOGS, LOGGER_INFO
				, DATABASE_INT_DB_CREATED_S, DATABASE_DB_NAME);
            return(database_create_tables());
        } else {
            error_handle(ERRORS_HANDLE_LOGS, LOGGER_INFO
				, DATABASE_INT_DB_CREATE_FAILED_S, DATABASE_DB_NAME);
            return(database_check_tables());
        }
    } else {
        error_handle(ERRORS_HANDLE_LOGS, LOGGER_ERROR
				, DATABASE_INT_DB_CREATE_ACCESS, DATABASE_DB_NAME);
    }

	binds = database_bind_setup(sql_connection);
    return(MYSQL_ERROR);
}

int database_create_tables()
{
    if (mysql_query(sql_connection, DATABASE_CREATE_TABLE) 
		== MYSQL_SUCCESS) {
     error_handle(ERRORS_HANDLE_LOGS, LOGGER_INFO
		, DATABASE_INT_TABLE_CREATED_S, DATABASE_TABLE_NAME);
    } else {
        error_handle(ERRORS_HANDLE_LOGS, LOGGER_ERROR
				, DATABASE_INT_TABLE_CREATE_FAILED_SS
				, DATABASE_DB_NAME
				, mysql_error(sql_connection));
        return(MYSQL_ERROR);
    }
    return(MYSQL_SUCCESS);
}

int database_check_tables()
{
    int query   = 
        mysql_query(sql_connection, DATABASE_TABLE_CHECKEXISTS);

    MYSQL_RES *result = mysql_store_result(sql_connection);
    mysql_free_result(result); 
    // required to clear the session for next query

    if (query == MYSQL_SUCCESS) {
       error_handle(ERRORS_HANDLE_LOGS, LOGGER_INFO
		, DATABASE_INT_TABLE_FOUND_S, DATABASE_TABLE_NAME);
    } else {
        error_handle(ERRORS_HANDLE_LOGS, LOGGER_INFO
		, DATABASE_INT_TABLE_NOT_FOUND_S, DATABASE_TABLE_NAME);
        return(database_create_tables());
    }
    return(MYSQL_SUCCESS);
}

int database_table_insertrow(char *query, MYSQL_BIND *bind, size_t count)
{
    MYSQL_STMT *stmt = mysql_stmt_init(sql_connection);
    if(stmt == NULL)
        return(MYSQL_ERROR);

    int result  = mysql_stmt_prepare(stmt, query , strlen(query));

    if(result){
        mysql_stmt_close(stmt);
        return(MYSQL_ERROR);
    }
	int bind_count	= mysql_stmt_param_count(stmt);
	if(bind_count != count)
		return(MYSQL_ERROR);

    result  = mysql_stmt_bind_param(stmt, bind);

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

// int database_table_query_results(MYSQL_STMT *statment)
// {
//     int result  = mysql_stmt_fetch(statment);
//     if(result){
//         mysql_stmt_close(statment);
//         return(MYSQL_ERROR);
//     }
//     return(MYSQL_SUCCESS);
// }

/**
 * "this function should not be used in production or non-debug builds"
 * should only be used to print all out the records returned
 * by running a sql select like statement
 */
void __database_query_print_dbg
	(MYSQL_STMT *stmt, database_table_bind_s bind)
{   
	printf("hello hi how are you\n");
    while (mysql_stmt_fetch(stmt) == 0)
    {
		int column_index	= 
			database_bind_column_index(bind, TABLE1_FI_COLUMN_FOLDER_NAME);
		char *buffer	= bind.fields[column_index].buffer;
        printf("what the fuck, %s\n", buffer);
    }
	printf("%s is the sql error\n", mysql_stmt_error(stmt));
}

MYSQL_STMT *database_table_query
	(char *query, MYSQL_BIND *bind_in ,MYSQL_BIND *bind_out)
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
    result  = mysql_stmt_bind_result(stmt, bind_out);
    if(result){
        mysql_stmt_close(stmt);
        return(NULL);
    }
    result  = mysql_stmt_bind_param(stmt, bind_in);
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

/* functions to help with binding memory for mysql in C */

/*
 * this function sets up the mysql binding params 
 * before we will be able to use mysql prepared statements
 */
database_table_bind_s database_bind_setup(MYSQL *mysql)
{
	int value	= mysql_query(mysql, DATABASE_BIND_QUERY);
	if(value != 0){
		error_handle(ERRORS_HANDLE_STDOLOG, LOGGER_CATASTROPHIC
			, MYSQLBIND_QUERY_FAILED, mysql_error(mysql));
		exit(SERVER_DATABASE_FAILURE);
	}
	int columns	= mysql_field_count(mysql);
	/* since we have LIMIT 1 in sql query, store result is fine */
	MYSQL_RES *result	= mysql_store_result(mysql); 
	if(result == NULL) {
		error_handle(ERRORS_HANDLE_STDOLOG, LOGGER_CATASTROPHIC
			, MYSQLBIND_QUERY_RESULT_FAILED, mysql_error(mysql));
		exit(SERVER_DATABASE_FAILURE);
	}

	MYSQL_FIELD	 *column	= NULL;
	database_table_bind_s table_binds	= {0};

	database_bind_fields_s	*bind_fields	= 
		m_malloc(sizeof(database_bind_fields_s) * columns
			, MEMORY_FILE_LINE);
	table_binds.fields	= bind_fields;
	table_binds.bind_params	= 
		m_malloc(sizeof(MYSQL_BIND) * columns, MEMORY_FILE_LINE);
	table_binds.bind_param_count	= columns;

	int iterator	= 0;
	do{
		column	= mysql_fetch_field(result);
		if(column == NULL) 
			break;
		database_bind_fields_s field	= database_bind_field(*column);
		bind_fields[iterator++] = field;

		error_handle(ERRORS_HANDLE_LOGS, LOGGER_DEBUG
			, MYSQLBIND_QUERY_COLUMN_DISCOVERED
			, field.table_length, field.table_name
			, field.name_length, field.name);
	} while(column != NULL);
	mysql_free_result(result);

	table_binds.table	= database_bind_table(&table_binds);
	database_bind_copybind(&table_binds);

	return(table_binds);
}

size_t database_bind_column_index
	(database_table_bind_s bind_table, string_s column_name)
{
	hash_table_bucket_s bucket	= hash_table_get(bind_table.table
		, column_name.address, column_name.length);
	if(bucket.is_occupied && bucket.key != 0) {
		size_t index	= (size_t)bucket.value;
		return(index);
	}else return(-1);
}

MYSQL_BIND *database_bind_some
	(database_table_bind_s bind_table, string_s *columns, int count)
{
	if(count > bind_table.bind_param_count) {
		error_handle(ERRORS_HANDLE_LOGS, LOGGER_ERROR
			, MYSQLBIND_COLUMN_COUNT_ERROR
			, bind_table.bind_param_count
			, count);
		return NULL;
	}

	MYSQL_BIND *bind	= 
		m_malloc(sizeof(MYSQL_BIND) * count, MEMORY_FILE_LINE);
	for(int i=0; i < count; ++i) {
		string_s string	= columns[i];
		hash_table_bucket_s bucket	= 
			hash_table_get(bind_table.table, string.address, string.length);
		if(bucket.key	== 0 || bucket.value == 0) {
			error_handle(ERRORS_HANDLE_LOGS, LOGGER_ERROR
				, MYSQLBIND_COLUMN_NOT_FOUND
				, string.length, string.address
				, string.length);
			return NULL;
		}
		MYSQL_BIND *bind_l	= (bind_table.bind_params + (size_t)bucket.value);
		memcpy(bind + i, bind_l, sizeof(MYSQL_BIND));
	}
	return(bind);
}

void database_bind_free(database_table_bind_s bind_table)
{
	for(size_t i=0; i < bind_table.bind_param_count; ++i)
	{
		database_bind_fields_s field	= bind_table.fields[i];
		m_free(field.name, MEMORY_FILE_LINE);
		m_free(field.table_name, MEMORY_FILE_LINE);
		m_free(field.buffer, MEMORY_FILE_LINE);
	}
	m_free(bind_table.fields, MEMORY_FILE_LINE);
	m_free(bind_table.bind_params, MEMORY_FILE_LINE);
	hash_table_free(bind_table.table);
}

hash_table_s database_bind_table(database_table_bind_s *bind_table)
{
	hash_table_s table	= hash_table_inits();
	for(size_t i=0; i < bind_table->bind_param_count; ++i){
		database_bind_fields_s *field	= (bind_table->fields + i);

		hash_table_bucket_s bucket = {0};
		bucket.key	= field->name;
		bucket.key_len	= field->name_length;
		bucket.value	= (char*)i;
		bucket.value_len	= 0;
		hash_table_add(&table, bucket);
	}
	return(table);
}

void database_bind_set_flags(database_bind_fields_s *column
	, size_t flags)
{
	column->is_numeric	= FLAG_ISSET(flags & NUM_FLAG);
	column->is_not_null	= FLAG_ISSET(flags & NOT_NULL_FLAG);
	column->is_primary	= FLAG_ISSET(flags & PRI_KEY_FLAG);
	column->is_unique	= FLAG_ISSET(flags & UNIQUE_KEY_FLAG);
	column->is_multikey	= FLAG_ISSET(flags & MULTIPLE_KEY_FLAG);
	column->is_binary	= FLAG_ISSET(flags & BINARY_FLAG);
	column->is_autoincr	= FLAG_ISSET(flags & AUTO_INCREMENT_FLAG);
	column->has_no_defaults	= FLAG_ISSET(flags & NO_DEFAULT_VALUE_FLAG);
}

database_bind_fields_s database_bind_field(MYSQL_FIELD field)
{
	database_bind_fields_s column	= {0};
	column.flags	= field.flags;
	column.name_length	= field.name_length;
	column.table_length	= field.table_length;
	column.type	= field.type;
	column.decimals	= field.decimals;
	column.charsetnr	= field.charsetnr;
	column.buffer_length	= field.length;
	database_bind_set_flags(&column, field.flags);
	size_t buffer_length	= field.length;
	if(column.is_numeric)	
		buffer_length	= sizeof(long long);
	column.table_name	= m_malloc(field.table_length + 1, MEMORY_FILE_LINE);
	column.name	= m_malloc(field.name_length + 1, MEMORY_FILE_LINE);
	column.buffer	= m_malloc(buffer_length, MEMORY_FILE_LINE);
	memcpy(column.name, field.name, field.name_length + 1);
	memcpy(column.table_name, field.table, field.table_length + 1);
	column.is_init	= TRUE;
	return(column);
}

void database_bind_copybind(database_table_bind_s *table)
{
	for(int i=0; i< table->bind_param_count; ++i){
		database_bind_fields_s *field	= (table->fields + i);

		MYSQL_BIND	bind	= {0};
		bind.buffer_type  = field->type;
  		bind.buffer   = field->buffer;
  		bind.is_null  = &field->is_null;
  		bind.length   = &field->length;
  		bind.error    = &field->is_error;
		bind.buffer_length	= field->buffer_length;
		table->bind_params[i]	= bind;
	}
}