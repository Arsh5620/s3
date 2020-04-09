#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "database.h"
#include "../logger/logs.h"
#include "../general/defines.h"
#include "../general/binarysearch.h"
#include "../memdbg/memory.h"
#include "../errors/errorhandler.h"

static MYSQL *sql_connection;
static database_table_bind_s binds;

struct config_parse config_property[5] = {
	STRUCT_CONFIG_PARSE("database", CONFIG_DATABASE
		, database_connection_s, db, CONFIG_TYPE_STRING)
	, STRUCT_CONFIG_PARSE("machine", CONFIG_MACHINE
		, database_connection_s, host, CONFIG_TYPE_STRING)
	, STRUCT_CONFIG_PARSE("password", CONFIG_PASSWORD
		, database_connection_s, passwd, CONFIG_TYPE_STRING)
	, STRUCT_CONFIG_PARSE("port", CONFIG_PORT
		, database_connection_s, port, CONFIG_TYPE_INT)
	, STRUCT_CONFIG_PARSE("username", CONFIG_USERNAME
		, database_connection_s, user, CONFIG_TYPE_STRING)
};

MYSQL *database_get_handle()
{
	return(sql_connection);
}

/* if you need more full binds you can use database_bind_setup
 * if you need partial binds you can copy global bind using
 * database_bind_some_copy. If you copy binds it will be your
 * responsibility to free those binds when not in use. 
 * ** please don't call free on global bind **
 * ** make sure to call database_bind_table_clean before use **
 */
database_table_bind_s database_get_global_bind()
{
	return(binds);
}

// after database_init is called a static variable sql_connection
// is initialized to the connection information and a mysql db 
// connection is attempted. 
int database_init(char *config_file)
{
	database_connection_s connect = {0};
	config_parse_files(config_file
		, config_property, CONFIG_COUNT, (char*)&connect);

	error_handle(ERRORS_HANDLE_LOGS, LOGGER_DEBUG
		, PROTOCOL_MYSQL_LOGIN_INFO_SISS
		, connect.host, connect.port
		, connect.user
		, connect.db);

	if(sql_connection != 0) return(MYSQL_SUCCESS);

	if (mysql_library_init(0, NULL, NULL))
	{
		error_handle(ERRORS_HANDLE_LOGS, LOGGER_ERROR
			, DATABASE_MYSQL_LIB_INIT_FAILED);
		exit(SERVER_DATABASE_FAILURE);
	}

	sql_connection 	= mysql_init(NULL);
	if (sql_connection == NULL) 
	{
		error_handle(ERRORS_HANDLE_LOGS, LOGGER_ERROR
			, DATABASE_MYSQL_INIT_FAILED);
		exit(SERVER_DATABASE_FAILURE);
	}

	if (NULL == mysql_real_connect(sql_connection
		, connect.host , connect.user
		, connect.passwd , connect.db
		, connect.port , NULL , 0))
	{
	   	error_handle(ERRORS_HANDLE_LOGS, LOGGER_ERROR
			, DATABASE_MYSQL_AUTH_FAILED_S
			, mysql_error(sql_connection));
		exit(SERVER_DATABASE_FAILURE);
	}
	error_handle(ERRORS_HANDLE_LOGS, LOGGER_INFO
			, DATABASE_MYSQL_CONNECTED);

	if(database_verify_integrity() != MYSQL_SUCCESS)
	{
		error_handle(ERRORS_HANDLE_LOGS, LOGGER_CATASTROPHIC
			, DATABASE_INTEGRITY_FAILED);
		exit(SERVER_DATABASE_INTEGRITY);
	}

	database_bind_init_global();
	return(MYSQL_SUCCESS);
}

void database_bind_init_global()
{
	binds = database_bind_setup(sql_connection, DATABASE_TABLE_FI_BIND);
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
	return(MYSQL_ERROR);
}

int database_create_tables()
{
	if (mysql_query(sql_connection, DATABASE_TABLE_FI_CREATE) 
		== MYSQL_SUCCESS) {
	 error_handle(ERRORS_HANDLE_LOGS, LOGGER_INFO
		, DATABASE_INT_TABLE_CREATED_S, DATABASE_TABLE_FI_NAME);
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
		mysql_query(sql_connection, DATABASE_TABLE_FI_CHECKEXISTS);

	MYSQL_RES *result = mysql_store_result(sql_connection);
	mysql_free_result(result); 
	// required to clear the session for next query

	if (query == MYSQL_SUCCESS) {
	   error_handle(ERRORS_HANDLE_LOGS, LOGGER_INFO
		, DATABASE_INT_TABLE_FOUND_S, DATABASE_TABLE_FI_NAME);
	} else {
		error_handle(ERRORS_HANDLE_LOGS, LOGGER_INFO
		, DATABASE_INT_TABLE_NOT_FOUND_S, DATABASE_TABLE_FI_NAME);
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


char database_table_rowexists(MYSQL_STMT *stmt)
{   
	char row_exists	= FALSE;
	while (mysql_stmt_fetch(stmt) == 0)
	{
		row_exists	= TRUE;
		// int column_index	= 
		// 	database_bind_column_index(bind, TABLE1_FI_COLUMN_FOLDER_NAME);
		// char *buffer	= bind.fields[column_index].buffer;
	}
	return(row_exists);
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
database_table_bind_s database_bind_setup(MYSQL *mysql, char *select_query)
{
	int value	= mysql_query(mysql, select_query);

	/**
	 *  if mysql_query return a value other than ZERO,
	 *  it means an error occured 
	 */
	if (value != 0)
	{
		error_handle(ERRORS_HANDLE_STDOLOG, LOGGER_CATASTROPHIC
			, MYSQLBIND_QUERY_FAILED, mysql_error(mysql));
		exit(SERVER_DATABASE_FAILURE);
	}

	/**
	 * after we have received our result set, we need to know
	 * the number of columns that are in the table
	 */
	int columns	= mysql_field_count(mysql);
	assert(columns != 0);

	MYSQL_RES *result	= mysql_store_result(mysql); 
	if (result == NULL) 
	{
		error_handle(ERRORS_HANDLE_STDOLOG, LOGGER_CATASTROPHIC
			, MYSQLBIND_QUERY_RESULT_FAILED, mysql_error(mysql));
		exit(SERVER_DATABASE_FAILURE);
	}

	MYSQL_FIELD	 *column	= NULL;
	database_table_bind_s table_binds	= {0};
	database_bind_allocate(&table_binds, columns);

	int iterator	= 0;
	while (column = mysql_fetch_field(result), column != NULL)
	{
		if (table_binds.table_name.address	== NULL)
		{
			STRING_S_MALLOC(table_binds.table_name
				, column->table, column->table_length);
		}

		database_bind_fields_s field	= database_bind_field(*column);
		table_binds.fields[iterator++] = field;

		error_handle(ERRORS_HANDLE_LOGS, LOGGER_DEBUG
			, MYSQLBIND_QUERY_COLUMN_DISCOVERED
			, table_binds.table_name.length, table_binds.table_name.address
			, field.name.length, field.name.address);
	}

	mysql_free_result(result);

	table_binds.hash_table	= database_bind_maketable(&table_binds);
	database_bind_linkfields(&table_binds);

	return(table_binds);
}

database_table_bind_s database_bind_select_copy
	(database_table_bind_s source, string_s *columns, int count)
{
	error_handle(ERRORS_HANDLE_LOGS, LOGGER_DEBUG
		, MYSQLBIND_BIND_COPY_REQUEST
		, count);

	for(size_t i = 0; i < count; ++i){
		string_s string	= columns[i];
		error_handle(ERRORS_HANDLE_LOGS, LOGGER_DEBUG
		, MYSQLBIND_BIND_COPY_REQUEST_INFO
		, string.length, string.address
		, string.length);
	}

	database_table_bind_s dest	= {0};

	if(count > source.count) {
		error_handle(ERRORS_HANDLE_LOGS, LOGGER_ERROR
			, MYSQLBIND_COLUMN_COUNT_ERROR
			, source.count
			, count);
		dest.error	= FAILED;
		return dest;
	}

	database_bind_allocate(&dest, count);

	for(int i=0; i < count; ++i) {
		string_s string	= columns[i];
		hash_table_bucket_s bucket	= 
			hash_table_get(source.hash_table
			, (hash_input_u){.address = string.address}, string.length);

		if(bucket.key.address == NULL) {
			error_handle(ERRORS_HANDLE_LOGS, LOGGER_ERROR
				, MYSQLBIND_COLUMN_NOT_FOUND
				, string.length, string.address
				, string.length);
			dest.error	= FAILED;
			return dest;
		}

		size_t index	= bucket.value.number;
		// MYSQL_BIND *source_bind	= (source.bind_params + index);
		// MYSQL_BIND *dest_bind	= (dest.bind_params + i);

		// memcpy(dest_bind, source_bind, sizeof(MYSQL_BIND));
		dest.fields[i]	= database_bind_field_copy(source.fields[index]);	
	}

	dest.hash_table	= database_bind_maketable(&dest);
	database_bind_linkfields(&dest);
	return(dest);
}


void database_bind_allocate(database_table_bind_s *bind, size_t columns)
{
	size_t field_size	= sizeof(database_bind_fields_s) * columns;
	size_t bind_size	= sizeof(MYSQL_BIND) * columns;		
	bind->fields		= m_malloc(field_size, MEMORY_FILE_LINE);
	bind->bind_params	= m_malloc(bind_size, MEMORY_FILE_LINE);
	bind->count	= columns;
}

size_t database_bind_column_index
	(database_table_bind_s bind_table, string_s column_name)
{
	hash_table_bucket_s bucket	= hash_table_get(bind_table.hash_table
		,(hash_input_u){.address = column_name.address} 
		, column_name.length);
	if(bucket.is_occupied && bucket.key.address != NULL) {
		size_t index	= bucket.value.number;
		return(index);
	}else return(-1);
}

void database_bind_table_clean(database_table_bind_s bind_table)
{
	for(size_t i = 0; i < bind_table.count; ++i)
	{
		database_bind_fields_s *field	= bind_table.fields + i;
		memset(field->buffer.address, 0, field->buffer.length);
		field->length	= 0;
		field->is_null	= 0;
		field->is_unsigned	= 0;
		field->is_error	= 0;
	}
}

void database_bind_free(database_table_bind_s bind)
{
	if(bind.table_name.address)
		m_free(bind.table_name.address, MEMORY_FILE_LINE);

	for(size_t i=0; i < bind.count; ++i)
	{
		database_bind_fields_s field	= bind.fields[i];
		m_free(field.name.address, MEMORY_FILE_LINE);
		m_free(field.buffer.address, MEMORY_FILE_LINE);
	}
	m_free(bind.fields, MEMORY_FILE_LINE);
	m_free(bind.bind_params, MEMORY_FILE_LINE);
	hash_table_free(bind.hash_table);

	error_handle(ERRORS_HANDLE_LOGS, LOGGER_DEBUG
			, MYSQLBIND_BIND_FREE
			, bind.count);
}

hash_table_s database_bind_maketable(database_table_bind_s *bind_table)
{
	hash_table_s table	= hash_table_init(10, 1);
	for(size_t i=0; i < bind_table->count; ++i)
	{
		database_bind_fields_s *field	= (bind_table->fields + i);

		hash_table_bucket_s bucket = {0};
		bucket.key.address	= field->name.address;
		bucket.key_len		= field->name.length;
		bucket.value.number	= i;
		bucket.value_len	= 0;
		hash_table_add(&table, bucket);
	}
	return(table);
}

database_bind_field_flags_s database_bind_set_flags(size_t flags)
{
	database_bind_field_flags_s flags_s;
	flags_s.is_numeric	= FLAG_ISSET(flags, NUM_FLAG);
	flags_s.is_not_null	= FLAG_ISSET(flags, NOT_NULL_FLAG);
	flags_s.is_primary	= FLAG_ISSET(flags, PRI_KEY_FLAG);
	flags_s.is_unique	= FLAG_ISSET(flags, UNIQUE_KEY_FLAG);
	flags_s.is_multikey	= FLAG_ISSET(flags, MULTIPLE_KEY_FLAG);
	flags_s.is_binary	= FLAG_ISSET(flags, BINARY_FLAG);
	flags_s.is_autoincr	= FLAG_ISSET(flags, AUTO_INCREMENT_FLAG);
	flags_s.has_no_defaults	= FLAG_ISSET(flags, NO_DEFAULT_VALUE_FLAG);
	return(flags_s);
}

database_bind_fields_s database_bind_field_copy
	(database_bind_fields_s src)
{
	database_bind_fields_s dest	= {0};
	memcpy(&dest, &src, sizeof(database_bind_fields_s));
	dest.buffer	=	database_bind_buffer_set(src.buffer.length, src.flags);
	STRING_S_MALLOC(dest.name, src.name.address, src.name.length);
	return(dest);
}

string_s database_bind_buffer_set(long length, unsigned int flags)
{
	size_t buffer_length	= length;
	if(FLAG_ISSET(flags, NUM_FLAG))	
		buffer_length	= sizeof(long long);

	string_s buffer	= {0};
	buffer.address	= m_calloc(buffer_length, MEMORY_FILE_LINE);
	buffer.length	= buffer_length;
	return(buffer);
}

database_bind_fields_s database_bind_field(MYSQL_FIELD field)
{
	database_bind_fields_s column	= {0};
	column.flags	= field.flags;
	column.type		= field.type;

	column.buffer	= database_bind_buffer_set(field.length, field.flags);
	STRING_S_MALLOC(column.name, field.name, field.name_length);
	column.init_complete	= TRUE;
	return(column);
}

void database_bind_linkfields(database_table_bind_s *table)
{
	for(int i=0; i< table->count; ++i)
	{
		database_bind_fields_s *field	= (table->fields + i);
		MYSQL_BIND	bind	= {0};
		bind.buffer_type	= field->type;
  		bind.buffer   = field->buffer.address;
  		bind.is_null  = &field->is_null;
  		bind.length   = &field->length;
  		bind.error    = &field->is_error;
		bind.buffer_length	= field->buffer.length;
		table->bind_params[i]	= bind;
	}
}

void database_bind_data_copy(MYSQL_BIND *bind, string_s string)
{
	memcpy(bind->buffer, string.address, string.length);
	*(bind->length) = string.length;
}