#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "./database.h"
#include "../general/binarysearch.h"
#include "../memdbg/memory.h"
#include "../output/output.h"

static MYSQL *sql_connection;
static database_table_bind_s binds;

data_keys_s configs[] = {
	DBP_KEY("database", CONFIG_DATABASE)
	, DBP_KEY("machine", CONFIG_MACHINE)
	, DBP_KEY("password", CONFIG_PASSWORD)
	, DBP_KEY("port", CONFIG_PORT)
	, DBP_KEY("username", CONFIG_USERNAME)
};

MYSQL *database_get_handle()
{
	return(sql_connection);
}

database_table_bind_s database_get_global_bind()
{
	return(binds);
}

int database_setup_login(data_result_s result, database_connection_s *conninfo)
{
	char *username	= data_get_key_value(result.hash, CONFIG_USERNAME);
	return (FAILED);
}	

// After database_init is called a static variable sql_connection
// is initialized to the connection information and a mysql db 
// connection is attempted. 
int database_init(char *config_file)
{
	database_connection_s connect = {0};

	data_result_s result	= data_parse_files(config_file
		, configs, sizeof(configs)/sizeof(data_keys_s));

	database_setup_login(result, &connect);

	output_handle(OUTPUT_HANDLE_LOGS, LOGGER_LEVEL_DEBUG
		, PROTOCOL_MYSQL_LOGIN_INFO
		, connect.host, connect.port
		, connect.user
		, connect.db);

	if (sql_connection != 0)
	{
		return(MYSQL_SUCCESS);
	}

	if (mysql_library_init(0, NULL, NULL))
	{
		output_handle(OUTPUT_HANDLE_LOGS
			, LOG_EXIT_SET(LOGGER_LEVEL_CATASTROPHIC, DATABASE_MYSQL_LIBINIT)
			, DATABASE_MYSQL_LIB_INIT_FAILED);
	}

	sql_connection 	= mysql_init(NULL);
	if (sql_connection == NULL) 
	{
		output_handle(OUTPUT_HANDLE_LOGS
			, LOG_EXIT_SET(LOGGER_LEVEL_CATASTROPHIC, DATABASE_MYSQL_INIT)
			, DATABASE_MYSQL_INIT_FAILED);
	}

	if (NULL == mysql_real_connect(sql_connection
		, connect.host , connect.user
		, connect.passwd , connect.db
		, connect.port , NULL , 0))
	{
	   	output_handle(OUTPUT_HANDLE_LOGS
		   	, LOG_EXIT_SET(LOGGER_LEVEL_CATASTROPHIC, DATABASE_LOGIN_FAIL)
			, DATABASE_MYSQL_AUTH_FAILED
			, mysql_error(sql_connection));
	}

	output_handle(OUTPUT_HANDLE_LOGS, LOGGER_LEVEL_INFO
			, DATABASE_MYSQL_CONNECTED);
	
	data_free(result);

	if (database_verify_integrity() != MYSQL_SUCCESS)
	{
		output_handle(OUTPUT_HANDLE_LOGS
			, LOG_EXIT_SET(LOGGER_LEVEL_CATASTROPHIC
			, DATABASE_INTEGRITY_CHECK_FAIL)
			, DATABASE_INTEGRITY_FAILED);
	}

	binds = database_bind_setup(sql_connection, DATABASE_TABLE_FI_BIND);
	return (MYSQL_SUCCESS);
}

int database_verify_integrity()
{
	output_handle(OUTPUT_HANDLE_LOGS, LOGGER_LEVEL_INFO
			, DATABASE_INTEGRITY_CHECK);
	output_handle(OUTPUT_HANDLE_LOGS, LOGGER_LEVEL_INFO
			, DATABASE_INTEGRITY_PING);

	int connection_enabled  = mysql_ping(sql_connection);
	if (connection_enabled == 0) 
	{ 
		output_handle(OUTPUT_HANDLE_LOGS, LOGGER_LEVEL_INFO
			, DATABASE_CONNECTED_SERVER
			, mysql_get_server_info(sql_connection));
	}

	if (MYSQL_SUCCESS == mysql_query(sql_connection
		, DATABASE_CREATE_DATABASE))
	{
		size_t  result  = mysql_affected_rows(sql_connection);

		if(MYSQL_SUCCESS != 
				mysql_select_db(sql_connection, DATABASE_DB_NAME)) 
		{
			output_handle(OUTPUT_HANDLE_LOGS, LOGGER_LEVEL_ERROR
				, DATABASE_INT_DB_SELECT_FAILED
				, mysql_error(sql_connection));
			return (MYSQL_ERROR);
		}

		if (result)
		{
			output_handle(OUTPUT_HANDLE_LOGS, LOGGER_LEVEL_INFO
				, DATABASE_INT_DB_CREATED, DATABASE_DB_NAME);
			return (database_create_tables());
		} 
		else 
		{
			output_handle(OUTPUT_HANDLE_LOGS, LOGGER_LEVEL_INFO
				, DATABASE_INT_DB_CREATE_FAILED, DATABASE_DB_NAME);
			return (database_check_tables());
		}
	} 
	else 
	{
		output_handle(OUTPUT_HANDLE_LOGS, LOGGER_LEVEL_ERROR
				, DATABASE_INT_DB_CREATE_ACCESS, DATABASE_DB_NAME);
	}
	return(MYSQL_ERROR);
}

int database_create_tables()
{
	if (MYSQL_SUCCESS == mysql_query(sql_connection, DATABASE_TABLE_FI_CREATE))
	{
		output_handle(OUTPUT_HANDLE_LOGS, LOGGER_LEVEL_INFO
			, DATABASE_INT_TABLE_CREATED, DATABASE_TABLE_FI_NAME);
	}
	else
	{
		output_handle(OUTPUT_HANDLE_LOGS, LOGGER_LEVEL_ERROR
			, DATABASE_INT_TABLE_CREATE_FAILED
			, DATABASE_DB_NAME
			, mysql_error(sql_connection));
		return(MYSQL_ERROR);
	}
	return(MYSQL_SUCCESS);
}

int database_check_tables()
{
	int query	= mysql_query(sql_connection, DATABASE_TABLE_FI_CHECKEXISTS);
	MYSQL_RES *result = mysql_store_result(sql_connection);
	mysql_free_result(result); 

	// required to clear the session for next query

	if (query == MYSQL_SUCCESS) 
	{
		output_handle(OUTPUT_HANDLE_LOGS, LOGGER_LEVEL_INFO
			, DATABASE_INT_TABLE_FOUND, DATABASE_TABLE_FI_NAME);
	} 
	else 
	{
		output_handle(OUTPUT_HANDLE_LOGS, LOGGER_LEVEL_INFO
			, DATABASE_INT_TABLE_NOT_FOUND, DATABASE_TABLE_FI_NAME);
		return(database_create_tables());
	}
	return(MYSQL_SUCCESS);
}

int database_table_call1(MYSQL_STMT *stmt, string_s query
	, MYSQL_BIND *bind, int bind_count)
{
	int result	= mysql_stmt_prepare(stmt, query.address, query.length);
	if (result > 0)
	{
		mysql_stmt_close(stmt);
		return (MYSQL_ERROR);
	}

	result	= mysql_stmt_param_count(stmt);
	if (result != bind_count)
	{
		mysql_stmt_close(stmt);
		return (MYSQL_ERROR);
	}

	result  = mysql_stmt_bind_param(stmt, bind);
	if (result > 0)
	{
		mysql_stmt_close(stmt);
		return (MYSQL_ERROR);
	}
	return(MYSQL_SUCCESS);
}

// @ return value, will return -1 for error, and any other value for success
int database_table_insert(int (*database_function)(MYSQL_STMT *)
	, string_s query , MYSQL_BIND *bind, size_t count)
{
	MYSQL_STMT *stmt = mysql_stmt_init(sql_connection);
	if (stmt == NULL)
	{
		return (-1);
	}

	if (database_table_call1(stmt, query, bind, count) == MYSQL_ERROR)
	{
		return(-1);
	}

	int result  = mysql_stmt_execute(stmt);
	if (result > 0)
	{
		mysql_stmt_close(stmt);
		return(-1);
	}
	
	result	= mysql_affected_rows(sql_connection) > 0 ? TRUE: FALSE;
	if (result == FALSE)
	{
		mysql_stmt_close(stmt);
		return (-1);
	}
	
	if (database_function != NULL)
	{
		result	= database_function(stmt);
	}
	mysql_stmt_close(stmt);
	return (result);
}

int database_table_row_exists(MYSQL_STMT *stmt)
{   
	boolean row_exists	= FALSE;
	while (mysql_stmt_fetch(stmt) == 0)
	{
		row_exists	= TRUE;
	}
	return(row_exists);
}

// @return value is -1 for error unlike most other functions
// which return MYSQL_ERROR on failure. 
// the function will return the rows affected on success
int database_table_stmt(string_s query
	, MYSQL_BIND *bind_in, uint bind_in_count)
{
	MYSQL_STMT *stmt	= mysql_stmt_init(sql_connection);
	if (stmt == NULL)
	{
		return (-1);
	}

	int result = database_table_call1(stmt, query, bind_in, bind_in_count);
	if (result != MYSQL_SUCCESS)
	{
		return (-1);
	}

	result  = mysql_stmt_execute(stmt);
	if (result)
	{
		mysql_stmt_close(stmt);
		return(-1);
	}

	result	= mysql_stmt_affected_rows(stmt);
	mysql_stmt_close(stmt);
	return(result);
}

//@return -1 for error, other values for success
int database_table_query(int (*database_function)(MYSQL_STMT *)
	, string_s query, MYSQL_BIND *bind_in, uint bind_in_count
	, MYSQL_BIND *bind_out)
{
	MYSQL_STMT *stmt	= mysql_stmt_init(sql_connection);
	if (stmt == NULL)
	{
		return(-1);
	}
	
	int result	= database_table_call1(stmt, query, bind_in, bind_in_count);
	if (result == MYSQL_ERROR)
	{
		return(-1);
	}

	result  = mysql_stmt_bind_result(stmt, bind_out);
	if (result)
	{
		mysql_stmt_close(stmt);
		return(-1);
	}

	result  = mysql_stmt_execute(stmt);
	if(result)
	{
		mysql_stmt_close(stmt);
		return(-1);
	}

	if (database_function != NULL)
	{
		result	= database_function(stmt);
	}

	mysql_stmt_close(stmt);
	return(result);
}

/* functions to help with binding memory for mysql in C */

/*
 * this function sets up the mysql binding params 
 * before we will be able to use mysql prepared statements
 */
database_table_bind_s database_bind_setup(MYSQL *mysql, char *select_query)
{
	int query	= mysql_query(mysql, select_query);

	/**
	 *  if mysql_query return a value other than ZERO,
	 *  it means an error occured 
	 */
	if (query != 0)
	{
		output_handle(OUTPUT_HANDLE_BOTH
			, LOG_EXIT_SET(LOGGER_LEVEL_CATASTROPHIC
			, DATABASE_BIND_QUERY_FAIL)
			, MYSQLBIND_QUERY_FAILED
			, mysql_error(mysql));
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
		output_handle(OUTPUT_HANDLE_BOTH
			, LOG_EXIT_SET(LOGGER_LEVEL_CATASTROPHIC
			, DATABASE_BIND_QUERY_FAIL)
			, MYSQLBIND_QUERY_RESULT_FAILED, mysql_error(mysql));
	}

	MYSQL_FIELD	 *column	= NULL;
	database_table_bind_s table_binds	= {0};
	database_bind_allocate(&table_binds, columns);

	int iterator	= 0;
	while (column = mysql_fetch_field(result), column != NULL)
	{
		if (table_binds.table_name.address	== NULL)
		{
			table_binds.table_name	= string_new_copy(column->table
				, column->table_length);
		}

		database_bind_fields_s field	= database_bind_field(column, NULL);
		table_binds.fields[iterator++] = field;

		output_handle(OUTPUT_HANDLE_LOGS, LOGGER_LEVEL_DEBUG
			, MYSQLBIND_QUERY_COLUMN_DISCOVERED
			, table_binds.table_name.length, table_binds.table_name.address
			, field.name.length, field.name.address);
	}

	mysql_free_result(result);

	table_binds.hash_table	= database_bind_maketable(&table_binds);
	database_bind_link_fields(&table_binds);

	return(table_binds);
}

database_table_bind_s database_bind_select_copy
	(database_table_bind_s source, string_s *columns, int count)
{
	output_handle(OUTPUT_HANDLE_LOGS, LOGGER_LEVEL_DEBUG
		, MYSQLBIND_BIND_COPY_REQUEST
		, count);

	for (size_t i = 0; i < count; ++i)
	{
		string_s string	= columns[i];
		output_handle(OUTPUT_HANDLE_LOGS, LOGGER_LEVEL_DEBUG
			, MYSQLBIND_BIND_COPY_REQUEST_INFO
			, string.length, string.address
			, string.length);
	}

	database_table_bind_s dest	= {0};

	if (count > source.count)
	{
		output_handle(OUTPUT_HANDLE_LOGS, LOGGER_LEVEL_ERROR
			, MYSQLBIND_COLUMN_COUNT_ERROR
			, source.count
			, count);
		dest.error	= FAILED;
		return dest;
	}

	database_bind_allocate(&dest, count);

	for (int i=0; i < count; ++i) 
	{
		string_s string	= columns[i];
		hash_input_u search	= (hash_input_u) {
			.address = string.address
		};

		hash_table_bucket_s bucket	= 
			hash_table_get(source.hash_table, search, string.length);

		if (bucket.key.address == NULL) 
		{
			output_handle(OUTPUT_HANDLE_LOGS, LOGGER_LEVEL_ERROR
				, MYSQLBIND_COLUMN_NOT_FOUND
				, string.length, string.address
				, string.length);
			dest.error	= FAILED;
			return dest;
		}

		size_t index	= bucket.value.number;
		dest.fields[i]	= database_bind_field(NULL, (source.fields + index));	
	}

	dest.hash_table	= database_bind_maketable(&dest);
	database_bind_link_fields(&dest);
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

// returns index of the column if found, and -1 if not found
size_t database_bind_column_index(database_table_bind_s bind_table
	, string_s column_name)
{
	hash_input_u search	= (hash_input_u) {
		.address = column_name.address
	};

	hash_table_bucket_s bucket	= hash_table_get(bind_table.hash_table
		, search, column_name.length);

	if (bucket.is_occupied && bucket.key.address != NULL) 
	{
		size_t index	= bucket.value.number;
		return (index);
	}
	else
	{
		return (-1);
	}
}

void database_bind_clean(database_table_bind_s bind_table)
{
	for (size_t i = 0; i < bind_table.count; ++i)
	{
		database_bind_fields_s *field	= (bind_table.fields + i);
		memset(field->buffer.address, 0, field->buffer.length);

		field->length	= 0;
		field->is_null	= 0;
		field->is_unsigned	= 0;
		field->is_error	= 0;
	}
}

void database_bind_free(database_table_bind_s bind)
{
	if (bind.table_name.address)
	{
		m_free(bind.table_name.address, MEMORY_FILE_LINE);
	}

	for (size_t i=0; i < bind.count; ++i)
	{
		m_free(bind.fields[i].buffer.address, MEMORY_FILE_LINE);
	}

	m_free(bind.fields, MEMORY_FILE_LINE);
	m_free(bind.bind_params, MEMORY_FILE_LINE);

	hash_table_free(bind.hash_table);
	output_handle(OUTPUT_HANDLE_LOGS, LOGGER_LEVEL_DEBUG
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

// you would need to set either the field OR the src
// src has more precedence over field, so if both are set, src will be used
database_bind_fields_s database_bind_field(MYSQL_FIELD *field
	, database_bind_fields_s *src)
{
	database_bind_fields_s column	= {0};
	size_t buffer_length	= 0, name_length	= 0;
	char *name_address	= 0;
	if (src != NULL)
	{
		memcpy(&column, src, sizeof(database_bind_fields_s));
		buffer_length	= src->buffer.length;
		name_length	= src->name.length;
		name_address	= src->name.address;
	}
	else 
	{
		column.flags	= field->flags;
		column.type		= field->type;
		buffer_length	= field->length;
		if (FLAG_ISSET(field->flags, NUM_FLAG))
		{	
			buffer_length	= sizeof(long long);
		}
		name_length	= field->name_length;
		name_address	= field->name;
	}

	ulong total_allocation	= buffer_length + name_length;
	char *address	= m_calloc(total_allocation, MEMORY_FILE_LINE);

	string_s buffer	= {
		.address	= address			
		, .length		= buffer_length
	};
	column.buffer	= buffer;
	string_s name = {
		.address	= buffer.address + buffer_length
		, .length	= name_length
	};
	column.name	= name;
	memcpy(column.name.address, name_address, name_length);
	
	column.init_complete	= TRUE;
	return(column);
}

void database_bind_link_fields(database_table_bind_s *table)
{
	for (int i=0; i< table->count; ++i)
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

int database_bind_add_data(database_table_bind_s bind_table
	, string_s column_name, string_s data)
{
	int column_index	= database_bind_column_index(bind_table, column_name);
	if (column_index == -1) 
	{
		return(MYSQL_ERROR);
	}

	MYSQL_BIND	*bind	= bind_table.bind_params + column_index;
	memcpy(bind->buffer, data.address, data.length);

	*(bind->length) = data.length;
	return(MYSQL_SUCCESS);
}